#include "rightframe.h"
#include "ui_rightframe.h"


RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);

    int yOffset = 0;

    connect(ui->alarm_button, &QPushButton::clicked, this, &RightFrame::alarmPage);
    connect(ui->event_button, &QPushButton::clicked, this, &RightFrame::eventPage);

    connect(ui->event_button, &QPushButton::clicked, this, &RightFrame::addNewWidget);


    ui->widget_1->setVisible(false);



    ui->scrollAreaWidgetContents->setMinimumHeight(10);

}

RightFrame::~RightFrame()
{
    delete ui;
    for(auto& widget: eventWidgets) {
        delete widget;
    }
}

/*
데이터 받기 - CAM 번호, 경고 종류, 경고 시간
이벤트 위젯 작성
이벤트 위젯 위치 설정 - 새 위젯은 0, 120, 이전 위젯은 0, 220, ...
*/

void RightFrame::eventPage() {
    ui->scrollArea->setVisible(true);
    ui->search->setVisible(true);
    ui->trans_alarm->setVisible(false);
    ui->trans_off->setVisible(true);
    ui->trans_on->setVisible(false);
    ui->event_on->setVisible(true);
    ui->event_off->setVisible(false);
}
void RightFrame::alarmPage() {
    ui->trans_alarm->setVisible(true);
    ui->scrollArea->setVisible(false);
    ui->search->setVisible(false);
    ui->trans_on->setVisible(true);
    ui->trans_off->setVisible(false);
    ui->event_off->setVisible(true);
    ui->event_on->setVisible(false);
}


void RightFrame::addNewWidget() {
    // 위젯 다 밀기
    for(auto& widget: eventWidgets) {
        widget->move(0, widget->y() + 100);
    }

    // 새로운 위젯 생성
    QWidget *newWidget = new QWidget(ui->scrollAreaWidgetContents);
    eventWidgets.push_back(newWidget);

    newWidget->setFixedSize(300, 100);
    newWidget->move(0, 0);

    newWidget->setStyleSheet(
        "background-color: transparent;"
        "border: none;"
        );

    // 위젯 안에 요소 추가
    QLabel *cam_label = new QLabel("CAM 00", newWidget);
    cam_label->setGeometry(25, 10, 75, 20);
    cam_label->setStyleSheet(
        "color: rgb(255, 255, 255);"
        "background-color: transparent;"
        "border: none;" "font-size: 800 30px;"
        );
    QLabel *icon = new QLabel("", newWidget);
    icon->setGeometry(30, 42, 16, 16);
    icon->setStyleSheet(
        "background-color: transparent;"
        "image: url(:/images/image/closing.png);"
        "border: none;"
        );
    static int i = 0;
    //QLabel *text_label = new QLabel("alarm text", newWidget);
    QLabel *text_label = new QLabel(QString::fromStdString(std::to_string(i++)), newWidget);
    text_label->setGeometry(52, 43, 130, 16);
    text_label->setStyleSheet(
        "color: rgb(176, 176, 176);"
        "background-color: transparent;"
        "border: none;" "font-size: 15px;"
        );
    QLabel *time_label = new QLabel("2025.00.00 00:00:00", newWidget);
    time_label->setGeometry(45, 65, 160, 16);
    time_label->setStyleSheet(
        "color: rgb(176, 176, 176);"
        "background-color: transparent;"
        "border: none;" "font-size: 13px;"
        );
    QFrame *line = new QFrame(newWidget);
    line->setGeometry(14, 90, 267, 1);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("background-color: rgb(44, 44, 44);");


    newWidget->show();

    // 다음 위젯 위치 계산
    yOffset += 100;

    // scrollAreaWidgetContents 높이 조정
    ui->scrollAreaWidgetContents->setMinimumHeight(yOffset);
    ui->scrollAreaWidgetContents->setMaximumHeight(yOffset);

}

