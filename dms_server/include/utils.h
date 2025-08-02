#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <vector>

// ************************ 편의 기능 함수 ************************
// 눈 랜드마크 이용해서 EAR (Eye Aspect Ratio) 계산해서 반환하는 함수
double computeEAR(const dlib::full_object_detection& s, int idx);
// 로그 출력 함수
void writeLog(std::string log);
// 제스처 점선 표시 함수
void drawGestureZones(cv::Mat& frame);
// ****************************************************************

// ********************* 소켓 입출력 함수 *************************
// fd와 연결된 소켓으로부터 암호화된 1바이트 명령어 읽은 후 반환하는 함수
uint8_t readEncryptedCommand(int fd);
// fd와 연결된 소켓으로부터 암호화된 1바이트 명령어 논블로킹으로 읽은 후 반환하는 함수
uint8_t readEncryptedCommandNonBlock(int fd);
// fd와 연결된 소켓에 buf에 저장된 데이터 len 바이트를 써주는 함수
int writeNBytes(int fd, const void* buf, int len);
// fd와 연결된 소켓에 buf에 저장된 프레임 데이터를 암호화 하여 써주는 함수
int writeEncryptedFrame(int fd, const std::vector<uchar>& buf);
// fd와 연결된 소켓에 double 데이터를 암호화하여 써주는 함수
int writeEncryptedData(int fd, uint8_t protocol, double data);
// fd와 연결된 소켓에 command 암호화하여 써주는 함수
int writeEncryptedCommand(int fd, uint8_t command);

int readEncryptedMessage(int fd, std::string& str);
int readEncryptedMessageNonBlocking(int fd, std::string &outStr);
// ****************************************************************

// ************* OpenSSL 이용한 암호화, 복호화 함수 *****************
// aes256_cbc 방식으로 암호화 하는 함수
bool aes_encrypt(const unsigned char* plaintext, int plaintext_len, const unsigned char* key, const unsigned char* iv, unsigned char* ciphertext, int* ciphertext_len);
// aes256_cbc 방식으로 복호화 하는 함수
bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int* plaintext_len);
// ****************************************************************

// 얼굴 사각형 그려주는 함수
void drawFaceRect(cv::Mat& frame, dlib::rectangle& faceRect);

#endif // UTILS_H