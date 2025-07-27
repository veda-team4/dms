#include "loginpage.h"
#include "ui_loginpage.h"
#include <QMessageBox>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginPage)
{
    ui->setupUi(this);

    connect(ui->loginButton, &QPushButton::clicked, this, [this]() {
        QString id = ui->idEdit->text();
        QString pw = ui->passwordEdit->text();
        if (id == "admin" && pw == "admin") {
            emit loginSuccess();
        }
        else {
            QMessageBox::warning(this, "로그인 실패", "아이디 또는 비밀번호가 올바르지 않습니다.");
        }
    });
}

LoginPage::~LoginPage()
{
    delete ui;
}
