#include "rightframe.h"
#include "ui_rightframe.h"

RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);
}

RightFrame::~RightFrame()
{
    delete ui;
}

/*
데이터 받기 - CAM 번호, 경고 종류, 경고 시간
이벤트 위젯 작성
이벤트 위젯 위치 설정 - 새 위젯은 0, 120, 이전 위젯은 0, 220, ...

이벤트, 알람 탭 버튼 - 탭, 내용 모두 스위치
*/
