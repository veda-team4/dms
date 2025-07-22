# dms_co2 - SCD4x CO₂ Sensor Linux Kernel Module

Sensirion SCD40/SCD41 센서를 위한 Linux 커널 모듈입니다.  
CO₂ (ppm), 온도(°C), 습도(%) 정보를 측정하며, 유저 공간에서는 `/dev/dms_co2`를 통해 접근 가능합니다.

```bash
cat /dev/dms_co2
# 출력 예: 426 24.50 43.00 이런 식으로 CO₂(ppm), 온도(°C), 습도(%RH)를 한 줄로 받아볼 수 있습니다.
```

---

## 구성 파일

| 파일          | 설명 |
|---------------|------|
| `dms_co2.c`   | 센서 제어, CRC 검사, CO₂/온습도 측정, `/dev/dms_co2` 인터페이스 제공 |
| `Makefile`    | 커널 모듈 및 오버레이 자동 빌드/설치 스크립트 |
| `scd4x.dts`   | Device Tree Overlay 파일 (→ `scd4x.dtbo`로 컴파일됨) |

---

## 설치 및 사용 방법

아래 명령어들은 `Makefile`을 기준으로 합니다.

| 명령어 | 기능 |
|--------|------|
| `make` | 커널 모듈(`.ko`)만 빌드 |
| `make dtbo` | `.dts → .dtbo` 변환만 수행 |
| `make install` | 모듈 설치 + 오버레이 등록 + 자동 로딩 설정 |
| `make clean` | 빌드 산출물 제거 |
| `make uninstall` | 설치된 `.ko`, `.dtbo`, 설정 파일 제거 |
| `make uninstall clean install reboot` | 완전 초기화 후 재설치 + 재부팅 |

> 설치 후 시스템 재부팅을 통해 장치 노드(`/dev/dms_co2`)가 자동 생성됩니다.

---

## 코드 구조 요약

### `dms_co2.c`

#### ▸ `sensirion_crc8()`, `sensirion_bytes_to_u16()`
- Sensirion의 CRC8 알고리즘 구현 및 바이트 변환 함수

#### ▸ `scd4x_measure_single_shot()`
- 단일 측정 수행  
- 측정 명령 전송 → 5초 대기(msleep) → 결과 수신 및 파싱

#### ▸ `dms_co2_read()`
- 유저 공간에서 `read()` 호출 시 동작  
- 센서 측정 결과를 문자열로 변환해 반환

#### ▸ `probe()`, `remove()`
- I2C 디바이스 트리 매칭 후 자동 probe  
- `misc_register()`를 통해 `/dev/dms_co2` 노드 생성

---

## 작성자

**JSY**  

---
