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

    loadingGif = new QMovie(":/images/image/loading.gif");
    ui->loadingLabel->setMovie(loadingGif);

    ui->waitingFrame_2->hide();
}

CamFrame::~CamFrame()
{
    delete loadingGif;
    delete ui;
}

void CamFrame::connectRtsp(QString ip) {
    player = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(this);

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia) {
            ui->waitingFrame_2->hide();
            videoWidget->show();
            videoWidget->raise();
            loadingGif->stop();
        }
    });

    ui->waitingFrame_1->hide();
    ui->waitingFrame_2->show();
    ui->waitingFrame_2->raise();
    loadingGif->start();
    videoWidget->setGeometry(ui->videoLabel->geometry());

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
