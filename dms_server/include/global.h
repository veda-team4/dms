#ifndef GLOBAL_H
#define GLOBAL_H

#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <iostream>

// ---------------------- 전역 변수 ----------------------
extern cv::VideoCapture cap;

extern dlib::shape_predictor sp; // 얼굴 랜드마크 추출하는 객체
extern dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
extern bool hasFace; // 얼굴 감지 유무
extern std::mutex faceMutex; // 위 두 변수 위한 뮤텍스
extern cv::Mat sharedFrame; // 쓰레드에게 전달하는 프레임
extern std::mutex frameMutex; // 위 변수 위한 뮤텍스
extern cv::Mat rtspFrame;
extern std::mutex rtspFrameMutex;
extern std::atomic<bool> running; // 쓰레드 동작 제어 변수

extern std::vector<int> landmarkIdx; // 사용할 얼굴 랜드마크

extern std::chrono::time_point<std::chrono::steady_clock> lastLeftTime, lastRightTime, lastStretchTime;
extern std::chrono::time_point<std::chrono::steady_clock> leftTime, rightTime, stretchTime;
extern std::mutex timeMutex;

extern bool gestureLock;

extern int server_fd, client_fd; // 서버, 클라이언트 소켓 파일 디스크립터
// --------------------------------------------------------

#endif // GLOBAL_H