#include "driverpage.h"
#include "ui_driverpage.h"
#include "connectdialog.h"

DriverPage::DriverPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DriverPage)
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, &DriverPage::handleButtonClick);

    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &DriverPage::updateFrame);
}

DriverPage::~DriverPage()
{
    delete ui;
    cap.release();
}

void DriverPage::handleButtonClick() {
    if (!connected) {
        ConnectDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            QString url = dlg.getUrl();
            cap.open(url.toStdString());

            if (cap.isOpened()) {
                connected = true;
                ui->connectButton->setText("Disconnect");
                frameTimer->start(33);
            }
        }
    }
    else {
        frameTimer->stop();
        cap.release();
        connected = false;
        ui->connectButton->setText("Connect");
        ui->videoLabel->clear();
    }
}

void DriverPage::updateFrame() {
    if (!cap.isOpened()) return;

    cv::Mat frame;

    cap.read(frame);

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->videoLabel->setPixmap(QPixmap::fromImage(qimg));
}
