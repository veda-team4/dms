#include "deletedriverdialog.h"
#include "ui_deletedriverdialog.h"
#include <QString>
#include <QPushButton>

DeleteDriverDialog::DeleteDriverDialog(const CameraInfo& info, MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::DeleteDriverDialog)
{
    ui->setupUi(this);

    // 상단바 제거
    setWindowFlags(
        Qt::Dialog
        | Qt::FramelessWindowHint      // 타이틀바(프레임) 전부 없애기
        | Qt::WindowSystemMenuHint      // (필요하면) 시스템 메뉴 권한 유지
    );

    // label_2에 텍스트 설정
    QString text = QString(R"(
    <div style="font:300 12pt 'hanwhaGothic'; color: white; background: transparent;">
    <span style="color:#ff7f27; font:12pt 'hanwhaGothic';">%1</span>을 제거하시겠습니까?
    </div>
    )").arg(info.name);
    ui->label_2->setText(text);

    // 확인 버튼 → accept()
    connect(ui->pushButton, &QPushButton::clicked, this, &DeleteDriverDialog::accept);

    // 취소 버튼 → reject()
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DeleteDriverDialog::reject);

    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });
}

DeleteDriverDialog::~DeleteDriverDialog()
{
    delete ui;
}
