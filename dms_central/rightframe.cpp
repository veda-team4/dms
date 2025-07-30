#include "rightframe.h"
#include "ui_rightframe.h"


RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);
//event_alaam_button
    connect(ui->alarm_button, &QPushButton::clicked, this, &RightFrame::alarmPage);
    connect(ui->event_button, &QPushButton::clicked, this, &RightFrame::eventPage);


    //scrollArea = new QScrollArea(this);
}

RightFrame::~RightFrame()
{
    delete ui;
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



/*
RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);

    scrollArea = new QScrollArea(this);
    widget_1 = new QWidget(scrollArea);

    contentWidget->setLayout(layout);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);
    setCentralWidget(scrollArea);

    // 예시: 1초마다 새 위젯 추가
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RightFrame::addNewWidget);
    timer->start(1000);  // 1초마다
}

void RightFrame::addNewWidget() {
    // 새 라벨 생성
    QLabel *label = new QLabel(QString("New Widget #%1").arg(++count));
    layout->addWidget(label);

    // 스크롤 맨 아래로 이동
    QTimer::singleShot(0, [=]() {
        QScrollBar *bar = scrollArea->verticalScrollBar();
        bar->setValue(bar->maximum());
    });
}


void RightFrame::addNewWidget()
{
    //    CustomScrollArea* scrollArea = new QScrollArea;
    scrollArea->resize(320, 240);
    scrollArea->show();

    // 1초마다 새로운 버튼 추가
    QTimer* timer = new QTimer;
    QObject::connect(timer, &QTimer::timeout, [=]() {
        QPushButton* btn = new QPushButton("New Item");
        btn->resize(300, 40);
        //   scrollArea->addNewWidget(btn);
    });
    timer->start(1000);  // 1초마다 추가

}*/
