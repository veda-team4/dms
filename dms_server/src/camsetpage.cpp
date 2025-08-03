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
      writeEncryptedCommand(client_fd, Protocol::FACEOK);
    }
    else {
      writeEncryptedCommand(client_fd, Protocol::FACENOTOK);
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

    // 영상 품질 향상
    cv::Mat ycrcb;
    cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    // CLAHE 적용
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(channels[0], channels[0]);
    // 감마 보정 추가
    double gammaVal;
    cv::Scalar meanBrightness = cv::mean(channels[0]);
    // 밝기 구간별 감마 값 설정
    if (meanBrightness[0] < 40) {
        gammaVal = 0.5;   // 아주 어두움 → 강하게 밝게
    } else if (meanBrightness[0] < 80) {
        gammaVal = 0.7;   // 어두움 → 보통 밝게
    } else if (meanBrightness[0] < 120) {
        gammaVal = 0.9;   // 약간 어두움 → 약간 보정
    } else {
        gammaVal = 1.0;   // 충분히 밝음 → 보정 안 함
    }
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

    // 클라이언트에 프레임 전송하기
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);
    if (writeEncryptedFrame(client_fd, buf) == -1) {
      return -1;
    }
  }

  return 0;
}