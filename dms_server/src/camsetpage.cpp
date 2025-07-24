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

int camsetpage() {
  while (true) {
    // 클라이언트 측으로부터 STOP 프로토콜 수신 시 종료
    uint8_t protocol;
    protocol = readEncryptedCommandNonBlock(client_fd);
    if (protocol != Protocol::NOTHING) {
      if (protocol == Protocol::STOP) {
        writeLog(std::string("message from client: STOP"));
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

    if (localHasFace) {
      // 얼굴 사각형 그리기
      drawFaceRect(frame, faceRect);
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