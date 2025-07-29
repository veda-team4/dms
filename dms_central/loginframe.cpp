#include "loginframe.h"
#include "ui_loginframe.h"

LoginFrame::LoginFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::LoginFrame)
{
    ui->setupUi(this);
    setPlaceholders();

    connect(ui->login_pushButton_2, &QPushButton::clicked, this, &LoginFrame::onLoginClicked);
}

void LoginFrame::setPlaceholders()
{
    ui->id_lineEdit_2->setPlaceholderText(QStringLiteral("아이디"));
    ui->pw_lineEdit_2-> setPlaceholderText(QStringLiteral("비밀번호"));

    ui->pw_lineEdit_2->setEchoMode(QLineEdit::Password);
}

void LoginFrame::onLoginClicked()
{

    bool ok = (ui->id_lineEdit_2->text() == "admin"
               && ui->pw_lineEdit_2->text() == "veda1234");
    if (ok) {
        emit loginSucceeded(); // 로그인 성공 시그널
        this->close(); // 로그인 창 닫기
    } else {
        QMessageBox::warning(this, "로그인 실패", "아이디/비밀번호를 확인하세요");
    }

}

LoginFrame::~LoginFrame()
{
    delete ui;
}
