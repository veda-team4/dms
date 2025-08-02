#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <cstdint>

/*
통신 간에 사용할 프로토콜 정의

프로토콜은 두 가지 유형이 잇다.

1. [Type(1byte)][Len(4byte)][Data]
  -> 데이터를 송신해야 할 때 사용

2. [Command(1byte)]
  -> 명령 / 특정 상황 발생 알림 송신해야 할 때 사용

protocols.h 에는 첫 1바이트에 해당하는 Type / Command 를 정의한다
*/

namespace Protocol {
  enum Type : uint8_t {
    // SERVER <- CLIENT [COMMAND]
    STARTPAGE,
    CAMSET,
    CALIBRATE,
    CALIBRATE_OPENED,
    CALIBRATE_CLOSED,
    CALIBRATE_FINISH,
    MONITOR,
    REPORT,
    STOP,
    LOCK,
    UNLOCK,
    NOTHING,
    // SERVER -> CLIENT [TYPE]
    OPENEDEAR,
    CLOSEDEAR,
    EARTHRESHOLD,
    EYECLOSEDRATIO,
    FRAME,
    FACEOK,
    FACENOTOK,
    // SERVER -> CLIENT [COMMAND]
    HEADDROPPED,
    LEFT,
    RIGHT,
    STRETCH,
    MESSAGE
  };
}

#endif // PROTOCOLS_H
