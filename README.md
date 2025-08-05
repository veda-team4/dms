# DMS
<img width="307" height="164" alt="image" src="https://github.com/user-attachments/assets/a34b0743-dd09-4a02-9682-afb43a2b5d79" />


## DMS란 무엇인가?
DMS는 ***D***river ***M***onitoring ***S***ystem의 약자입니다. DMS는 대시보드에 장착된 카메라를 사용하여 운전자의 졸음을 감지하고, 이를 통해 운전자에게 오디오 경보와 경고등을 제공하여 집중력을 되찾도록 돕는 안전 시스템입니다.

이 프로젝트는 Raspberry Pi 4 Model B와 Raspberry Pi Camera Module 2를 사용하여 운전자의 눈이 감겨 있거나 머리가 숙여져 있는지를 실시간으로 감지합니다. 시스템이 졸음을 감지하면 LED와 스피커를 사용해 시각적, 청각적 경고를 제공하여 운전자를 깨웁니다.

추가적으로, 운전자가 졸음 상태임을 주변 차량에 알리기 위해 경고가 시작될 때 비상등도 동시에 켜집니다.

## 시스템 구조

### 하드웨어 구성
- **Raspberry Pi 4 Model B**
- **Raspberry Pi Camera Module 2**
- **LED (경고 표시)**
- **스피커 (경보음 출력)**
- **릴레이 모듈 (차량 비상등 제어)**

### 소프트웨어 구성
- **영상 처리 파이프라인**
  - OpenCV를 사용한 실시간 영상 입력
  - 얼굴 및 눈 랜드마크 추출 (dlib / MediaPipe / NCNN)
  - EAR(Eye Aspect Ratio) 계산 및 고개 숙임 감지
  - 졸음 여부 판단 알고리즘

- **알람 제어 로직**
  - 졸음 상태 감지 시 LED, 스피커 활성화
  - 비상등 제어 (GPIO + 릴레이)

- **멀티스레드 구조**
  - 카메라 캡처 스레드
  - 영상 분석 스레드
  - 알람 제어 스레드

---

### 하드웨어 구성도
<img width="676" height="395" alt="Screenshot 2025-08-05 at 5 13 36 PM" src="https://github.com/user-attachments/assets/cc7d380d-bf10-4a9a-8440-31e44db48471" />

### 소프트웨어 처리 흐름도
<img width="1921" height="979" alt="dms_diagram drawio (5)" src="https://github.com/user-attachments/assets/44932dd1-b359-4cda-a914-f561ba2ba85f" />

## 기능 요약
- **실시간 졸음 감지**
- **시각 + 청각 경고**
- **비상등 점멸로 외부 알림**
- **라즈베리파이 기반 저비용 구현**

---

## 설치 및 실행 방법

### 1. 환경 준비
- Raspberry Pi OS 설치
- OpenCV, dlib 또는 MediaPipe, NCNN 라이브러리 설치
- GPIO 제어를 위한 RPi.GPIO 라이브러리 설치

```bash
sudo apt update
sudo apt install python3-opencv python3-pip
pip3 install mediapipe RPi.GPIO
