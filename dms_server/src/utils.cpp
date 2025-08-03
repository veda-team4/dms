#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#include "utils.h"
#include "protocols.h"
#include "global.h"

static unsigned char key[33] = "abcdefghijklmnopqrstuvwxyz012345";

double computeEAR(const dlib::full_object_detection& s, int idx) {
  auto dist = [](const dlib::point& a, const dlib::point& b) {
    double dx = a.x() - b.x(), dy = a.y() - b.y();
    return std::sqrt(dx * dx + dy * dy);
    };
  double A = dist(s.part(idx + 1), s.part(idx + 5));
  double B = dist(s.part(idx + 2), s.part(idx + 4));
  double C = dist(s.part(idx), s.part(idx + 3));
  return (A + B) / (2.0 * C);
}

void writeLog(std::string log) {
  std::cout << "[Server] " << log << std::endl;
}

uint8_t readEncryptedCommand(int fd) {
  unsigned char iv[16];
  recv(fd, iv, 16, MSG_WAITALL); // IV 수신

  uint32_t ciphertext_len;
  recv(fd, &ciphertext_len, 4, MSG_WAITALL); // 길이 수신

  unsigned char ciphertext[32];
  recv(fd, ciphertext, ciphertext_len, MSG_WAITALL); // 암호문 수신

  unsigned char plaintext[32]; // 평문
  int plaintext_len;

  aes_decrypt(ciphertext, ciphertext_len, key, iv, plaintext, &plaintext_len);

  return plaintext[0];
}

uint8_t readEncryptedCommandNonBlock(int fd) {
  unsigned char iv[16];
  if (recv(fd, iv, 16, MSG_DONTWAIT) < 0) { // IV 수신
    return Protocol::NOTHING;
  }

  uint32_t ciphertext_len;
  recv(fd, &ciphertext_len, 4, MSG_WAITALL); // 길이 수신

  unsigned char ciphertext[32];
  recv(fd, ciphertext, ciphertext_len, MSG_WAITALL); // 암호문 수신

  unsigned char plaintext[32];
  int plaintext_len;

  aes_decrypt(ciphertext, ciphertext_len, key, iv, plaintext, &plaintext_len);
  ciphertext[plaintext_len] = '\0';

  return plaintext[0];
}

int writeEncryptedFrame(int fd, const std::vector<uchar>& buf) {
  // 1. 평문 만들기: protocol + len + frame data
  uint8_t protocol = Protocol::FRAME;
  uint32_t len = buf.size();
  std::vector<unsigned char> plaintext;

  plaintext.push_back(protocol);
  plaintext.insert(plaintext.end(),
    reinterpret_cast<unsigned char*>(&len),
    reinterpret_cast<unsigned char*>(&len) + 4);
  plaintext.insert(plaintext.end(), buf.begin(), buf.end());

  // 2. 암호화
  unsigned char iv[16];
  RAND_bytes(iv, sizeof(iv));

  unsigned char ciphertext[131072];
  int ciphertext_len;

  if (!aes_encrypt(plaintext.data(), plaintext.size(), key, iv, ciphertext, &ciphertext_len))
    return -1;

  // 3. 전송 구조: [IV(16)] + [암호문 길이(4)] + [암호문]
  if(writeNBytes(fd, iv, 16) == -1) return -1;
  if(writeNBytes(fd, &ciphertext_len, 4) == -1) return -1;
  if(writeNBytes(fd, ciphertext, ciphertext_len) == -1) return -1;

  return 0;
}

int writeEncryptedData(int fd, uint8_t protocol, double data) {
  // 1. 평문 만들기: protocol + len + double data
  uint32_t len = sizeof(data);
  std::vector<unsigned char> plaintext;

  plaintext.push_back(protocol);
  plaintext.insert(plaintext.end(), reinterpret_cast<unsigned char*>(&len),
    reinterpret_cast<unsigned char*>(&len) + 4);
  plaintext.insert(plaintext.end(), reinterpret_cast<unsigned char*>(&data),
    reinterpret_cast<unsigned char*>(&data) + len);

  // 2. 암호화
  unsigned char iv[16];
  RAND_bytes(iv, sizeof(iv));

  unsigned char ciphertext[64];
  int ciphertext_len;

  if (!aes_encrypt(plaintext.data(), plaintext.size(), key, iv, ciphertext, &ciphertext_len))
    return -1;

  // 3. 전송 구조: [IV(16)] + [암호문 길이(4)] + [암호문]
  if(writeNBytes(fd, iv, 16) == -1) return -1;
  if(writeNBytes(fd, &ciphertext_len, 4) == -1) return -1;
  if(writeNBytes(fd, ciphertext, ciphertext_len) == -1) return -1;

  return 0;
}

int writeEncryptedCommand(int fd, uint8_t command) {
  unsigned char iv[16];
  RAND_bytes(iv, sizeof(iv));  // 무작위 IV 생성

  // 평문 명령 1바이트 준비
  unsigned char plaintext[1] = { command };

  unsigned char ciphertext[64];
  int ciphertext_len;

  aes_encrypt(plaintext, 1, key, iv, ciphertext, &ciphertext_len);

  // 전송 구조: [IV(16)] + [암호문 길이(4)] + [암호문]
  if (writeNBytes(fd, iv, 16) == -1) return -1;
  if (writeNBytes(fd, &ciphertext_len, 4) == -1) return -1;
  if (writeNBytes(fd, ciphertext, ciphertext_len) == -1) return -1;

  return 0;
}

int writeNBytes(int fd, const void* buf, int len) {
  int totalWritten = 0;
  const char* buffer = (const char*)buf;

  while (totalWritten < len) {
    ssize_t bytesWritten = write(fd, buffer + totalWritten, len - totalWritten);

    if (bytesWritten < 0) {
      if (errno == EINTR) {
        continue;  // 시그널 인터럽트 시 다시 시도
      }
      perror("write");
      return -1;     // 쓰기 실패
    }

    if (bytesWritten == 0) {
      // write가 0을 반환하면 더 이상 쓸 수 없다는 뜻 → 실패
      return -1;
    }

    totalWritten += bytesWritten;
  }

  return totalWritten == len ? totalWritten : -1;
}

int readEncryptedMessage(int fd, std::string& str) {
    // 1. IV 읽기
    unsigned char iv[16];
    if (recv(fd, iv, 16, MSG_WAITALL) <= 0) {
        perror("recv IV");
        return -1;
    }

    // 2. 암호문 길이 읽기
    uint32_t ciphertext_len;
    if (recv(fd, &ciphertext_len, 4, MSG_WAITALL) <= 0) {
        perror("recv ciphertext_len");
        return -1;
    }

    // 안전성 체크 (예: 1MB 제한)
    if (ciphertext_len > 1048576) {
        std::cerr << "Ciphertext too large\n";
        return -1;
    }

    // 3. 암호문 읽기
    std::vector<unsigned char> ciphertext(ciphertext_len);
    if (recv(fd, ciphertext.data(), ciphertext_len, MSG_WAITALL) <= 0) {
        perror("recv ciphertext");
        return -1;
    }

    // 4. 복호화
    std::vector<unsigned char> plaintext(ciphertext_len + 32); // 패딩 고려
    int plaintext_len = 0;

    if (!aes_decrypt(ciphertext.data(), ciphertext_len, key, iv,
                     plaintext.data(), &plaintext_len)) {
        std::cerr << "AES decryption failed\n";
        return -1;
    }

    // 5. 결과 문자열로 변환
    str.assign(reinterpret_cast<char*>(plaintext.data()), plaintext_len);
    return 0;
}

int writeEncryptedMessage(int fd, std::string msg) {
    // 1. 평문 구성: protocol + len + string
    uint8_t protocol = Protocol::MESSAGE;
    uint32_t len = msg.size();

    std::vector<unsigned char> plaintext;
    plaintext.push_back(protocol);

    plaintext.insert(plaintext.end(),
                     reinterpret_cast<unsigned char*>(&len),
                     reinterpret_cast<unsigned char*>(&len) + 4);

    plaintext.insert(plaintext.end(),
                     reinterpret_cast<const unsigned char*>(msg.data()),
                     reinterpret_cast<const unsigned char*>(msg.data()) + msg.size());

    // 2. IV 생성
    unsigned char iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        perror("RAND_bytes");
        return -1;
    }

    // 3. 암호화
    std::vector<unsigned char> ciphertext(plaintext.size() + 32); // 패딩 여유
    int ciphertext_len = 0;

    if (!aes_encrypt(plaintext.data(), plaintext.size(), key, iv,
                     ciphertext.data(), &ciphertext_len)) {
        std::cerr << "AES encryption failed\n";
        return -1;
    }

    // 4. 전송 구조: [IV(16)] + [암호문 길이(4)] + [암호문]
    if (writeNBytes(fd, iv, 16) == -1) return -1;
    if (writeNBytes(fd, &ciphertext_len, 4) == -1) return -1;
    if (writeNBytes(fd, ciphertext.data(), ciphertext_len) == -1) return -1;

    return 0;
}

int readEncryptedMessageNonBlocking(int fd, std::string &outStr) {
    // 상태 변수들을 static 으로 저장 (호출 사이 상태 유지)
    static enum { READ_IV, READ_LEN, READ_DATA } state = READ_IV;
    static unsigned char iv[16];
    static uint32_t ciphertext_len = 0;
    static std::vector<unsigned char> ciphertext;
    static size_t bytesRead = 0;

    // ---- 1. IV 읽기 ----
    if (state == READ_IV) {
        ssize_t ret = recv(fd, iv + bytesRead, 16 - bytesRead, MSG_DONTWAIT);
        if (ret <= 0) {
            if (ret == 0) return 0; // 클라이언트 종료
            if (errno == EAGAIN || errno == EWOULDBLOCK) return -2; // 아직 데이터 없음
            perror("recv IV");
            return -1;
        }
        bytesRead += ret;
        if (bytesRead < 16) return -2; // 아직 덜 읽음
        state = READ_LEN;
        bytesRead = 0;
    }

    // ---- 2. 길이 읽기 ----
    if (state == READ_LEN) {
        ssize_t ret = recv(fd, reinterpret_cast<char*>(&ciphertext_len) + bytesRead,
                           4 - bytesRead, MSG_DONTWAIT);
        if (ret <= 0) {
            if (ret == 0) return 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return -2;
            perror("recv length");
            return -1;
        }
        bytesRead += ret;
        if (bytesRead < 4) return -2;
        state = READ_DATA;
        bytesRead = 0;
        ciphertext.resize(ciphertext_len);
    }

    // ---- 3. 암호문 읽기 ----
    if (state == READ_DATA) {
        ssize_t ret = recv(fd, ciphertext.data() + bytesRead,
                           ciphertext_len - bytesRead, MSG_DONTWAIT);
        if (ret <= 0) {
            if (ret == 0) return 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK) return -2;
            perror("recv data");
            return -1;
        }
        bytesRead += ret;
        if (bytesRead < ciphertext_len) return -2; // 아직 덜 읽음

        // 복호화
        std::vector<unsigned char> plaintext(ciphertext_len + 32);
        int plaintext_len = 0;
        if (!aes_decrypt(ciphertext.data(), ciphertext_len, key, iv,
                         plaintext.data(), &plaintext_len)) {
            std::cerr << "AES decryption failed\n";
            return -1;
        }

        outStr.assign(reinterpret_cast<char*>(plaintext.data()), plaintext_len);

        // 상태 초기화
        state = READ_IV;
        bytesRead = 0;
        ciphertext.clear();

        return 1; // 성공적으로 읽음
    }

    return -2;
}

bool aes_encrypt(const unsigned char* plaintext, int plaintext_len,
  const unsigned char* key, const unsigned char* iv,
  unsigned char* ciphertext, int* ciphertext_len) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return false;

  // 초기화 (AES-256-CBC, key/iv 설정)
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len;

  // 평문 암호화
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  *ciphertext_len = len;

  // 패딩 처리 및 마무리
  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  *ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return true;
}

bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len,
  const unsigned char* key, const unsigned char* iv,
  unsigned char* plaintext, int* plaintext_len) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) return false;

  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }

  int len;
  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  *plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
    EVP_CIPHER_CTX_free(ctx);
    return false;
  }
  *plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return true;
}

void drawGestureZones(cv::Mat& frame) {
  int lineSpacing = 10;   // 점선 간격
  int lineLength = 5;     // 점선 길이
  int thickness = 2;
  cv::Scalar color(33, 115, 243); // #F37321 → BGR

  int height = frame.rows;
  int width = frame.cols;

  int leftX = static_cast<int>(width * 0.2);
  int rightX = static_cast<int>(width * 0.8);

  for (int y = 0; y < height; y += lineSpacing) {
    // 왼쪽 점선
    cv::line(frame, cv::Point(leftX, y), cv::Point(leftX, std::min(y + lineLength, height - 1)), color, thickness);

    // 오른쪽 점선
    cv::line(frame, cv::Point(rightX, y), cv::Point(rightX, std::min(y + lineLength, height - 1)), color, thickness);
  }
}

void drawFaceRect(cv::Mat& frame, dlib::rectangle& faceRect) {
  int lineLen = 20;   // 코너 선 길이
    int thickness = 4;  // 선 두께
    cv::Scalar color(33, 115, 243); // #F37321 → BGR

    cv::Point tl(faceRect.left(), faceRect.top());          // 왼쪽 위
    cv::Point tr(faceRect.right(), faceRect.top());         // 오른쪽 위
    cv::Point bl(faceRect.left(), faceRect.bottom());       // 왼쪽 아래
    cv::Point br(faceRect.right(), faceRect.bottom());      // 오른쪽 아래

    // 왼쪽 위
    cv::line(frame, tl, cv::Point(tl.x + lineLen, tl.y), color, thickness);
    cv::line(frame, tl, cv::Point(tl.x, tl.y + lineLen), color, thickness);

    // 오른쪽 위
    cv::line(frame, tr, cv::Point(tr.x - lineLen, tr.y), color, thickness);
    cv::line(frame, tr, cv::Point(tr.x, tr.y + lineLen), color, thickness);

    // 왼쪽 아래
    cv::line(frame, bl, cv::Point(bl.x + lineLen, bl.y), color, thickness);
    cv::line(frame, bl, cv::Point(bl.x, bl.y - lineLen), color, thickness);

    // 오른쪽 아래
    cv::line(frame, br, cv::Point(br.x - lineLen, br.y), color, thickness);
    cv::line(frame, br, cv::Point(br.x, br.y - lineLen), color, thickness);
}