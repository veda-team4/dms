#include "topframe.h"
#include "ui_topframe.h"

TopFrame::TopFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::TopFrame),
    clockTimer(new QTimer(this))
{
    ui->setupUi(this);

    // 시간 설정 (1초 간격으로 updateTime() 호출)
    connect(clockTimer, &QTimer::timeout,
            this,       &TopFrame::updateTime);

    updateTime();
    clockTimer->start(1000);

    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });
}

void TopFrame::setAdminID(const QString &id)
{
    ui->admin_label->setText(id);
}

void TopFrame::startClock()
{
    if (!clockTimer->isActive()) {
        updateTime();
        clockTimer->start(1000);
    }
}

void TopFrame::updateTime()
{
    QString now = QTime::currentTime().toString("HH:mm:ss");
    ui->time_label->setText(now);
}

TopFrame::~TopFrame()
{
    delete ui;
}
