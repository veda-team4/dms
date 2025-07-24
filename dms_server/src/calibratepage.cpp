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
#include "protocols.h"
#include "utils.h"

// ********** 눈 크기 감지 관련 상수 **********
#define EAR_THRESH_VAL 0.30 // 감은 눈 EAR + (뜬 눈 EAR - 감은 눈 EAR) * X
// *******************************************

int calibrateEyes(double* ear, bool opened);

int calibratepage(double* thresholdEAR, double* openedEAR, double* closedEAR) {
  // 뜬 눈 EAR, 감은 눈 EAR
  while (true) {
    uint8_t protocol;
    protocol = readEncryptedCommandNonBlock(client_fd);
    if (protocol != Protocol::NOTHING) {
      if (protocol == Protocol::CALIBRATE_OPENED) {
        writeLog("message from client: CALIBRATE_OPENED");
        calibrateEyes(openedEAR, true);
      }
      else if (protocol == Protocol::CALIBRATE_CLOSED) {
        writeLog("message from client: CALIBRATE_CLOSED");
        calibrateEyes(closedEAR, false);
        // EAR threshold 값 계산
        *thresholdEAR = *closedEAR + (*openedEAR - *closedEAR) * EAR_THRESH_VAL;

        // 클라이언트에 EAR threshold 값 전송하기
        uint8_t protocol = Protocol::EARTHRESHOLD;
        if (writeEncryptedData(client_fd, protocol, *thresholdEAR) == -1) return -1;

        writeLog(std::string("Opened: " + std::to_string(*openedEAR)));
        writeLog(std::string("Closed: " + std::to_string(*closedEAR)));
        writeLog(std::string("Threshold: ") + std::to_string(*thresholdEAR));
      }
      else if (protocol == Protocol::STOP) {
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
    if (frame.empty()) continue;
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

    if (localHasFace) {
      // 얼굴 사각형 그리기
      drawFaceRect(frame, faceRect);

      // 랜드마크 그리기
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(frame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
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

int calibrateEyes(double* ear, bool opened) {
  double earSum = 0.0;
  int earCount = 0;

  // 클라이언트 측으로부터 finish 수신할 때까지 프레임 전송하며 EAR 계산
  while (true) {
    uint8_t protocol;
    protocol = readEncryptedCommandNonBlock(client_fd);
    if (protocol != Protocol::NOTHING) {
      // 클라이언트 측으로부터 finish 수신 시 while 문 빠져나감
      if (protocol == Protocol::CALIBRATE_FINISH) {
        writeLog("message from client: CALIBRATE_FINISH");
        break;
      }
      else {
        return -1;
      }
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) continue;
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

    if (localHasFace) {
      // 얼굴 사각형 그리기
      drawFaceRect(frame, faceRect);

      // 랜드마크 그리기
      dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame);
      dlib::full_object_detection landmarks = sp(dlibFrame, faceRect);

      for (int i = 0; i < landmarkIdx.size(); ++i) {
        dlib::point p = landmarks.part(landmarkIdx[i]);
        cv::circle(frame, cv::Point(p.x(), p.y()), 2, cv::Scalar(255, 0, 0), -1);
      }

      earSum += (computeEAR(landmarks, 36) + computeEAR(landmarks, 42)) / 2.0;
      ++earCount;
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

  *ear = earSum / earCount;

  // 클라이언트에 EAR 값 전송하기
  uint8_t protocol = (opened ? Protocol::OPENEDEAR : Protocol::CLOSEDEAR);
  uint32_t size = sizeof(*ear);
  if (writeEncryptedData(client_fd, protocol, *ear) == -1) return -1;

  return 0;
}
