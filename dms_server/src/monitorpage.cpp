#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include "threads.h"
#include "global.h"
#include "utils.h"
#include "protocols.h"

// ********** 눈 감김 감지 관련 상수 **********
#define BLINK_WINDOW_MS 1500 // 분석 시간 윈도우
#define INCREASE_THRESH  10 // 고개 움직임 감지 최소 값
#define MAX_DOWN_COUNT 2 // 몇번 카운트 해야 경고할 것인지
// ********************************************

int monitorpage(double thresholdEAR) {
  // 시작 시간 기록
  auto startTime = std::chrono::steady_clock::now();

  // 눈 감음 정보 저장 변수
  std::deque<std::pair<std::chrono::steady_clock::time_point, bool>> blinkHistory;
  unsigned long long closedCount = 0;
  double eyeClosedRatio = 0.0; // BLINK_WINDOW_MS 중에 눈 감은 비율

  // 고개 떨어짐 정보 저장 변수
  int downCount = 0; // 고개 떨어짐 횟수
  double earAvg = 0.0; // 양쪽 눈의 EAR 평균

  while (true) {
    // 클라이언트 측으로부터 "stop" 수신 시 종료
    uint8_t protocol;
    protocol = readEncryptedCommandNonBlock(client_fd);
    if (protocol != Protocol::NOTHING) {
      if (protocol == Protocol::STOP) {
        writeLog("message from client: STOP");
        return 0;
      }
      else if (protocol == Protocol::LOCK) {
        writeLog(std::string("message from client: LOCK"));
        gestureLock = true;
      }
      else if (protocol == Protocol::UNLOCK) {
        writeLog(std::string("message from client: UNLOCK"));
        gestureLock = false;
      }
      else {
        return -1;
      }
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) break;
    cv::flip(frame, frame, 1);

    // 얼굴 탐지 쓰레드를 위해 최신 프레임 공유
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      frame.copyTo(sharedFrame);
    }

    // 얼굴 사각형과 감지 유무 받아옴
    dlib::rectangle faceRect;
    bool localHasFace;
    {
      std::lock_guard<std::mutex> lock(faceMutex);
      localHasFace = hasFace;
      faceRect = biggestFaceRect;
    }

    // 얼굴 감지 성공시
    if (localHasFace) {
      // 68개의 얼굴 랜드마크 추출
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      // 얼굴 사각형 그리기
      drawFaceRect(frame, faceRect);

      // 랜드마크 이용해서 EAR 계산
      double earL = computeEAR(landmarks, 36);
      double earR = computeEAR(landmarks, 42);
      earAvg = (computeEAR(landmarks, 36) + computeEAR(landmarks, 42)) / 2.0;
      
      // 눈 감음 여부 계산. 감았을 시 closedCount 증가 및 경고 문구 출력
      bool isClosed = (earAvg < thresholdEAR);
      if (isClosed) {
        cv::putText(frame, "CLOSED",
          cv::Point(faceRect.left(), faceRect.top() - 10),
          cv::FONT_HERSHEY_SIMPLEX, 1.0,
          cv::Scalar(0, 0, 255), 2);
        }

      // { 현재 시간, 눈 감음 여부} 기록 / 1.5초 이전에는 떠있다고 체크 (비율 계산 안정성 위해)
      if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() <= 1500) {
        static int i = 0;
        blinkHistory.emplace_back(std::chrono::steady_clock::now(), false);
      }
      else {
        blinkHistory.emplace_back(std::chrono::steady_clock::now(), isClosed);
        if(isClosed) {
          ++closedCount;
        }
      }
        
      // 고개 떨어짐 계산. 얼굴 사각형 중앙 y 좌표로 계산
      static double prevFaceY = 1e50; // 이전 고개 좌표
      double currentFaceY = (faceRect.top() + faceRect.bottom()) / 2.0;
      double diff = currentFaceY - prevFaceY;
      prevFaceY = currentFaceY;

      // 이전 좌표와 비교해서 INCREASE_THRESH 보다 크게 증가하면 downCount 증가
      if (diff > INCREASE_THRESH && isClosed) {
        ++downCount;
      }
      // 이전 좌표와 비교해서 INCREASE_THRESH 보다 작게 증가하면 downCount 0 으로 초기화
      else if (diff < 0) {
        downCount = 0;
      }
    }
    else {
      blinkHistory.emplace_back(std::chrono::steady_clock::now(), false);
    }

    // 1.5초 간의 윈도우 초과한 항목 제거
    while (!blinkHistory.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - blinkHistory.front().first).count() > BLINK_WINDOW_MS) {
      if (blinkHistory.front().second) --closedCount;
      blinkHistory.pop_front();
    }

    // 1.5초간의 { 현재 시간, 눈 감음 여부 } Window 에서 눈 감김 비율 계산
    if (!blinkHistory.empty()) {
      eyeClosedRatio = static_cast<double>(closedCount) / blinkHistory.size();
    }

    // 눈 감김 비율 전송
    protocol = Protocol::EYECLOSEDRATIO;
    if (writeEncryptedData(client_fd, protocol, eyeClosedRatio) == -1) {
      return -1;
    }

    // 머리 떨어짐 감지 및 출력
    if (downCount >= MAX_DOWN_COUNT) {
      if (writeEncryptedCommand(client_fd, Protocol::HEADDROPPED) == -1) {
        return -1;
      }
    }

    {
      std::lock_guard<std::mutex> lock(timeMutex);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(rightTime - lastRightTime).count() > 0) {
        writeLog("Gesture: RIGHT");
        lastRightTime = rightTime;
        if (writeEncryptedCommand(client_fd, Protocol::RIGHT) == -1) {
          return -1;
        }
      }
    }

    {
      std::lock_guard<std::mutex> lock(timeMutex);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(leftTime - lastLeftTime).count() > 0) {
        writeLog("Gesture: LEFT");
        lastLeftTime = leftTime;
        if (writeEncryptedCommand(client_fd, Protocol::LEFT) == -1) {
          return -1;
        }
      }
    }

    {
      std::lock_guard<std::mutex> lock(timeMutex);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(stretchTime - lastStretchTime).count() > 0) {
        writeLog("Gesture: STRETCH");
        lastStretchTime = stretchTime;
        if (writeEncryptedCommand(client_fd, Protocol::STRETCH) == -1) {
          return -1;
        }
      }
    }

    if (!gestureLock) {
      drawGestureZones(frame);
    }

    // 클라이언트에 프레임 전송하기
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    if (writeEncryptedFrame(client_fd, buf) == -1) {
      return -1;
    }
  }

  return 0;
}