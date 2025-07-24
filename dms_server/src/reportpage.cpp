#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "threads.h"
#include "global.h"
#include "utils.h"
#include "protocols.h"

int reportpage() {
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

    // 제스처 검출 쓰레드를 위해 최신 프레임 공유
    {
      std::lock_guard<std::mutex> lock(frameMutex);
      frame.copyTo(sharedFrame);
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

  }
  return 0;
}