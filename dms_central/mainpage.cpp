#include "mainpage.h"
#include "ui_mainpage.h"
#include "connectdialog.h"
#include <opencv2/opencv.hpp>
#include <QTimer>

MainPage::MainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainPage)
{
    ui->setupUi(this);

    videoLabels = { ui->videoLabel_1, ui->videoLabel_2, ui->videoLabel_3, ui->videoLabel_4 };

    connect(ui->connectButton, &QPushButton::clicked, this, &MainPage::openConnectDialog);

    // 프레임 업데이트 타이머
    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &MainPage::updateFrames);
    frameTimer->start(33); // ~30fps
}

MainPage::~MainPage()
{
    delete ui;
    cap.release();
}

void MainPage::openConnectDialog() {
    ConnectDialog dlg(this);
    connect(&dlg, &ConnectDialog::connectToRtsp, this, &MainPage::addStream);
    dlg.exec();
}

void MainPage::addStream(const QString& url) {
    cv::VideoCapture* cap = new cv::VideoCapture(url.toStdString());
    if (!cap->isOpened()) {
        qDebug("RTSP error");
        delete cap;
        return;
    }
    captures.append(cap);
}

void MainPage::updateFrames() {
    for (int i = 0; i < captures.size() && i < videoLabels.size(); ++i) {
        cv::Mat frame;
        if (!captures[i]->read(frame)) continue;

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        videoLabels[i]->setPixmap(QPixmap::fromImage(qimg).scaled(videoLabels[i]->size(),
                                                                  Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation));
    }
}
