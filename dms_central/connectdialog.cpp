#include "connectdialog.h"
#include "ui_connectdialog.h"
#include <QPushButton>
#include <QString>

ConnectDialog::ConnectDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectDialog::onConnectClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::close);
    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });

    ui->comboBox->addItem(QIcon(":/images/image/camera.png"), "CAM 01 - 192.168.0.58", "192.168.0.58");
    ui->comboBox->addItem(QIcon(":/images/image/camera.png"), "CAM 02 - 192.168.0.72", "192.168.0.72");
    ui->comboBox->addItem(QIcon(":/images/image/camera.png"), "CAM 03");

}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

QString ConnectDialog::selectedIp() {
    return m_selectedIp;
}

void ConnectDialog::onConnectClicked() {
    m_selectedIp = ui->comboBox->currentData().toString();
    accept();
}
