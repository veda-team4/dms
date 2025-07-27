#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QPushButton>

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectDialog::onConnectButtonClicked);
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

void ConnectDialog::onConnectButtonClicked() {
    QString ip = ui->ipEdit->text();
    QString port = ui->portEdit->text();
    QString url = QString("rtsp://%1:%2/stream").arg(ip, port); // 실제 경로 맞게 수정
    emit connectToRtsp(url);
    accept(); // 모달 닫기
}
