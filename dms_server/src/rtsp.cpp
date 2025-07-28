#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <cstdio>
#include "global.h"
#include "utils.h"

// 전역
static FILE* ffmpegPipe = nullptr;

void pushFrameToRtsp();
void stopRtsp();

// --- MediaMTX로 RTSP 푸시 시작 ---
void startRtsp() {
    streaming = true;

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
    writeLog("join");
    stopRtsp();
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