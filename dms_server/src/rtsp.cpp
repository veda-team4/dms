#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "global.h"
#include "utils.h"
#include "protocols.h"

// 전역
static FILE* ffmpegPipe = nullptr;
extern double eyeClosedRatio;
extern std::chrono::steady_clock::time_point prevHeadTime;
extern std::chrono::steady_clock::time_point currentHeadTime;
extern std::chrono::steady_clock::time_point prevStretchTime;
extern std::chrono::steady_clock::time_point currentStretchTime;
extern std::atomic<bool> newMsg;
extern std::string msg;

void pushFrameToRtsp();
void stopRtsp();
void connectSocket();

// --- MediaMTX로 RTSP 푸시 시작 ---
void startRtsp() {
    // 소켓 연결 받기
    streaming = true;
    std::thread socketThread(connectSocket);
    socketThread.detach();
    
    // FFmpeg 명령어: rawvideo 입력 → H.264 인코딩 → RTSP 푸시
    const char* cmd =
    "ffmpeg -y -f rawvideo -pixel_format bgr24 -video_size 320x240 -framerate 30 -i - "
    "-c:v libx264 -preset ultrafast -tune zerolatency -f rtsp "
    "rtsp://127.0.0.1:8554/dms_stream";
    
    // FFmpeg 프로세스 실행
    ffmpegPipe = popen(cmd, "w");
    if (!ffmpegPipe) {
        writeLog("[RTSP] Failed to start FFmpeg process for MediaMTX push");
        return;
    }
    
    writeLog("[RTSP] Started pushing to MediaMTX (RTSP)");
    
    std::thread pushThread(pushFrameToRtsp);
    pushThread.join();
    stopRtsp();
}

void connectSocket() {
    int s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_fd < 0) {
        writeLog("socket error");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    // 3. 소켓 옵션 + 바인딩
    int opt = 1;
    setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(s_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s_fd);
        return;
    }

    // 4. 리스닝
    if (listen(s_fd, 1) < 0) {
        perror("listen");
        close(s_fd);
        return;
    }

    // 5. non-blocking 설정
    int flags = fcntl(s_fd, F_GETFL, 0);
    fcntl(s_fd, F_SETFL, flags | O_NONBLOCK);

    writeLog("Waiting for client connection on port 9000...");

    while (streaming) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int c_fd = accept(s_fd, (sockaddr*)&client_addr, &client_len);
        if (c_fd >= 0) {
            writeLog("[Socket] Client connected");

            while(streaming) {
                std::string outStr;
                char buf;
                int ret = readEncryptedMessageNonBlocking(c_fd, outStr);
                if (ret == 0) {
                    // 클라이언트 종료
                    writeLog("[Socket] Client disconnected");
                    close(c_fd);
                    c_fd = -1;
                    break;
                }
                else if (ret == 1) {
                    msg = outStr;
                    newMsg = true;
                } 
                uint8_t protocol = Protocol::EYECLOSEDRATIO;
                if (writeEncryptedData(c_fd, protocol, eyeClosedRatio) == -1) {
                    continue;
                }
                if (prevHeadTime < currentHeadTime) {
                    prevHeadTime = currentHeadTime;
                    if (writeEncryptedCommand(c_fd, Protocol::HEADDROPPED) == -1) {
                        continue;
                    }
                }
                if (prevStretchTime < currentStretchTime) {
                    prevStretchTime = currentStretchTime;
                    if (writeEncryptedCommand(c_fd, Protocol::STRETCH) == -1) {
                        continue;
                    }
                }

                usleep(100000);
            }
            close(c_fd);
        }
        sleep(1);
    }

    close(s_fd);
}

// --- 프레임 전송 ---
void pushFrameToRtsp() {
    while (streaming) {
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(rtspFrameMutex);
            if (rtspFrame.empty()) continue;
            rtspFrame.copyTo(frame);
        }

        // 영상 품질 향상
        cv::Mat ycrcb;
        cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> channels;
        cv::split(ycrcb, channels);
        // CLAHE 적용
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(channels[0], channels[0]);
        // 감마 보정 추가
        double gammaVal = 0.8;  // 기본 감마 값 (0.8 ~ 1.0 정도로 테스트 가능)
        // 1) 0~255 범위를 0~1로 정규화
        cv::Mat gammaCorrected;
        channels[0].convertTo(gammaCorrected, CV_32F, 1.0 / 255.0);
        // 2) 감마 보정 적용
        cv::pow(gammaCorrected, gammaVal, gammaCorrected);
        // 3) 다시 0~255로 변환
        gammaCorrected.convertTo(channels[0], CV_8U, 255.0);
        // 채널 합치고 BGR 복원
        cv::merge(channels, ycrcb);
        cv::cvtColor(ycrcb, frame, cv::COLOR_YCrCb2BGR);

        // 크기 조정
        cv::resize(frame, frame, cv::Size(320, 240));

        // raw BGR 데이터 전송
        fwrite(frame.data, 1, frame.total() * frame.elemSize(), ffmpegPipe);
        fflush(ffmpegPipe);
        std::this_thread::sleep_for(std::chrono::milliseconds(33)); // 30fps
    }
}

// --- 푸시 중지 ---
void stopRtsp() {
    if (ffmpegPipe) {
        pclose(ffmpegPipe);
        ffmpegPipe = nullptr;
        writeLog("[RTSP] Stopped pushing to MediaMTX");
    }
}