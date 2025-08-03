#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QString>

ConnectDialog::ConnectDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // 초기상태 스타일
    ui->status_label->setTextFormat(Qt::RichText);
    ui->status_label->setText(
        tr("연결할 카메라를 선택한 후 "
           "<span style='color:#CACACA; font-weight:bold;'>[연결]</span> 버튼을 눌러주세요.")
        );


    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectDialog::onConnectClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::close);
    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });
    connect(ui->comboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &ConnectDialog::onCameraSelected);

    ui->comboBox->addItem(" 카메라 선택 ", QString());
    for(auto& info: mainWindow->camerainfo) {
        QString name = info.name + QString(" - ") + info.ip;
        ui->comboBox->addItem(QIcon(":/images/image/camera.png"), name, info.ip);
    }
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

QString ConnectDialog::selectedIp() {
    return m_selectedIp;
}

QString ConnectDialog::selectedName() {
    return m_selectedName;
}

void ConnectDialog::onConnectClicked() {
    m_selectedIp = ui->comboBox->currentData().toString();
    for(auto& info: mainWindow->camerainfo) {
        if(info.ip == m_selectedIp) {
            m_selectedName = info.name;
        }
    }
    accept();
}

void ConnectDialog::onCameraSelected(int idx) {

    if(idx <= 0 || ui->comboBox->currentData().toString().isEmpty()){
        const QString plugIcon = QStringLiteral(":/images/image/plug.svg");

        ui->status_label->setTextFormat(Qt::RichText);
        ui->status_label->setText(
            tr("연결할 카메라를 선택한 후 "
               "<span style='color:#CACACA; font-weight:bold;'>[연결]</span> 버튼을 눌러주세요.")
            );

        ui->status_icon->setIcon(QIcon(plugIcon));
        ui->status_icon->setIconSize(QSize(16,16));
        return;
    }

    const QString camName = (ui->comboBox->currentText()).section(" - ", 0, 0).trimmed();
    const QString checkIcon = QStringLiteral(":/images/image/check.svg");

    ui->status_label->setText(
        tr("<span style='color:#F37321;'>%1</span>가 선택되었습니다. "
           "<span style='color:#CACACA; font-weight:bold;'>[연결]</span> 버튼을 눌러주세요.")
            .arg(camName)
        );

    ui->status_icon->setIcon(QIcon(checkIcon));
    ui->status_icon->setIconSize(QSize(24,22));
}
