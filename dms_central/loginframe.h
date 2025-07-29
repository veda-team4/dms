#ifndef LOGINFRAME_H
#define LOGINFRAME_H

#include <QFrame>
#include <QMessageBox>

namespace Ui {
class LoginFrame;
}

class LoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LoginFrame(QWidget *parent = nullptr);
    ~LoginFrame();

signals:
    void loginSucceeded(); // 로그인 성공 시그널

private slots:
    void onLoginClicked(); // 버튼 처리

private:
    Ui::LoginFrame *ui;
    void setPlaceholders();

};

#endif // LOGINFRAME_H
