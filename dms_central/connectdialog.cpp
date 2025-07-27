#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QPushButton>
#include <QString>

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

QString ConnectDialog::getUrl() {
    return QString("rtsp://%1:%2/mystream").arg(ip, port);
}

void ConnectDialog::onConnectButtonClicked() {
    ip = ui->ipEdit->text();
    port = ui->portEdit->text();
    accept(); // 모달 닫기
}
