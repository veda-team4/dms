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
    "rtsp://127.0.0.1:8554/mystream";
    
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
    
    close(server_fd);
    close(client_fd);
}

void connectSocket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        writeLog("socket error");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    // 3. 소켓 옵션 + 바인딩
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }

    // 4. 리스닝
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }

    // 5. non-blocking 설정
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    writeLog("Waiting for client connection on port 9000...");

    while (streaming) {
        /*
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0) {
            writeLog("[Socket] Client connected");

            while(streaming) {
                char buf;
                int ret = recv(client_fd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
                if (ret == 0) {
                    // 클라이언트 종료
                    writeLog("[Socket] Client disconnected");
                    close(client_fd);
                    client_fd = -1;
                    break;
                }
                uint8_t protocol = Protocol::EYECLOSEDRATIO;
                if (writeEncryptedData(client_fd, protocol, eyeClosedRatio) == -1) {
                    continue;
                }
                if (prevHeadTime < currentHeadTime) {
                    prevHeadTime = currentHeadTime;
                    if (writeEncryptedCommand(client_fd, Protocol::HEADDROPPED) == -1) {
                        continue;
                    }
                }
            }
            close(client_fd);
        }
        sleep(1);
    */
    }

    close(server_fd);
}

// --- 프레임 전송 ---
void pushFrameToRtsp() {
    while (streaming) {
        char buf;
        int ret = recv(client_fd, &buf, 1, MSG_PEEK | MSG_DONTWAIT);
        if (ret == 0) {
            // 연결 종료
            writeLog("Client disconnected");
            streaming = false;
            break;
        }
        
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(rtspFrameMutex);
            if (rtspFrame.empty()) continue;
            rtspFrame.copyTo(frame);
        }

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