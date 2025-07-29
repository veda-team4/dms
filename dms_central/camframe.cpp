#include "camframe.h"
#include "ui_camframe.h"
#include "connectdialog.h"

CamFrame::CamFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::CamFrame)
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, [=]() {
        ConnectDialog dlg(mainWindow, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString ip = dlg.selectedIp();
            connectRtsp(ip);
        }
    });
}

CamFrame::~CamFrame()
{
    delete ui;
}

void CamFrame::connectRtsp(QString ip) {
    player = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(this);

    videoWidget->setGeometry(ui->videoLabel->geometry());
    videoWidget->raise();
    videoWidget->show();

    player->setVideoOutput(videoWidget);

    QString rtspUrl = QString("rtsp://%1:8554/mystream").arg(ip);
    player->setSource(QUrl((rtspUrl)));
    player->play();
}

void CamFrame::disconnectRtsp() {
    if (player) {
        delete player;
    }
    if (videoWidget) {
        delete videoWidget;
    }
}
