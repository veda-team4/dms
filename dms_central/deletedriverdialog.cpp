#include "deletedriverdialog.h"
#include "ui_deletedriverdialog.h"
#include <QString>
#include <QPushButton>

DeleteDriverDialog::DeleteDriverDialog(const CameraInfo& info, MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::DeleteDriverDialog)
{
    ui->setupUi(this);

    // label_2에 텍스트 설정
    QString text = QString(R"(
    <div style="font: 12pt 'hanwhaGothic'; color: white; background: transparent;">
    <span style="color:#ff7f27; font-weight:bold;">%1</span>을 제거하시겠습니까?
    </div>
    )").arg(info.name);
    ui->label_2->setText(text);

    // 확인 버튼 → accept()
    connect(ui->pushButton, &QPushButton::clicked, this, &DeleteDriverDialog::accept);

    // 취소 버튼 → reject()
    connect(ui->pushButton_2, &QPushButton::clicked, this, &DeleteDriverDialog::reject);
}

DeleteDriverDialog::~DeleteDriverDialog()
{
    delete ui;
}
