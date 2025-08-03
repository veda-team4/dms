#include "adddriverdialog.h"
#include "ui_adddriverdialog.h"
#include <QString>
#include <QPushButton>

AddDriverDialog::AddDriverDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::AddDriverDialog)
{
    ui->setupUi(this);

    // 상단바 제거
    setWindowFlags(
        Qt::Dialog
        | Qt::FramelessWindowHint      // 타이틀바(프레임) 전부 없애기
        | Qt::WindowSystemMenuHint      // (필요하면) 시스템 메뉴 권한 유지
    );

    connect(ui->pushButton, &QPushButton::clicked, this, &AddDriverDialog::onConfirmClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AddDriverDialog::reject);
    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });
}

AddDriverDialog::~AddDriverDialog()
{
    delete ui;
}

CameraInfo AddDriverDialog::cameraInfo() const {
    return m_info;
}

void AddDriverDialog::onConfirmClicked() {
    m_info.name = ui->lineEdit->text();
    m_info.ip   = ui->lineEdit_2->text();
    accept();
}
