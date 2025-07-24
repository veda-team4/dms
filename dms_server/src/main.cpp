#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include "threads.h"
#include "functions.h"
#include "protocols.h"
#include "utils.h"

// ---------------------- 전역 변수 ----------------------
cv::VideoCapture cap;

dlib::shape_predictor sp; // 얼굴 랜드마크 추출하는 객체
dlib::rectangle biggestFaceRect; // 가장 큰 얼굴 사각형
bool hasFace = false; // 얼굴 감지 유무
std::mutex faceMutex; // 위 두 변수 위한 뮤텍스
cv::Mat sharedFrame; // 쓰레드에게 전달하는 프레임
std::mutex frameMutex; // 위 변수 위한 뮤텍스
std::atomic<bool> running = true; // 쓰레드 동작 제어 변수

std::vector<int> landmarkIdx = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59 }; // 사용할 얼굴 랜드마크

std::chrono::time_point<std::chrono::steady_clock> lastLeftTime, lastRightTime, lastStretchTime;
std::chrono::time_point<std::chrono::steady_clock> leftTime, rightTime, stretchTime;
std::mutex timeMutex;

bool gestureLock = true;

int server_fd, client_fd; // 서버, 클라이언트 소켓 파일 디스크립터
// --------------------------------------------------------

int main(void) {
  // 0. 소켓 PATH 설정
  char SOCKET_PATH[128] = { 0, };
  strcpy(SOCKET_PATH, getenv("HOME"));
  strcat(SOCKET_PATH, "/.dms_unix_socket");

  // 1. UNIX Domain Socket 생성
  server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket()");
    return -1;
  }

  // 2. 소켓 - 주소 연결
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
  unlink(SOCKET_PATH);

  if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind()");
    return -1;
  }

  // 3. 클라이언트 연결 대기
  listen(server_fd, 1);
  writeLog("Waiting for client ....");

  // 4. 클라이언트 연결
  client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    perror("accept");
    return -1;
  }
  writeLog("Client connected !");

  // 5. 카메라 열기
  cap.open(0, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    writeLog("Camera open failed");
    return -1;
  }
  cap.set(cv::CAP_PROP_FPS, 30);

  // 6. 얼굴 탐지 쓰레드 생성
  std::thread faceThread(runFaceDetectionThread);

  // 7. 제스처 감지 쓰레드 생성
  std::thread gestureThread(runGestureDetectionThread);

  // 8. 루프 돌며 클라이언트로부터 명령 받아서 실행
  double thresholdEAR, openedEAR, closedEAR;
  while (true) {
    uint8_t protocol;
    protocol = readEncryptedCommand(client_fd);
    if (protocol == Protocol::STARTPAGE) {
      writeLog("message from client: STARTPAGE");
      if (startpage() == -1) {
        writeLog("startpage error");
        break;
      }
    }
    else if (protocol == Protocol::CAMSET) {
      writeLog("message from client: CAMSET");
      if (camsetpage() == -1) {
        writeLog("camsetpage error");
        break;
      }
    }
    else if (protocol == Protocol::CALIBRATE) {
      writeLog("message from client: CALIBRATE");
      if (calibratepage(&thresholdEAR, &openedEAR, &closedEAR) == -1) {
        writeLog("calibrate error");
        break;
      }
    }
    else if (protocol == Protocol::MONITOR) {
      writeLog("message from client: MONITOR");
      if (monitorpage(thresholdEAR) == -1) {
        writeLog("monitor error");
        break;
      }
    }
    else if (protocol == Protocol::REPORT) {
      writeLog("message from client: REPORT");
      if (reportpage() == -1) {
        writeLog("report error");
        break;
      }
    }
    else if (protocol == Protocol::STOP) {
      break;
    }
  }

  running = false;
  // faceThread.join();
  gestureThread.join();
  close(client_fd);
  close(server_fd);
  unlink(SOCKET_PATH);
  return 0;
}