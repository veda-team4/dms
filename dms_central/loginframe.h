#ifndef LOGINFRAME_H
#define LOGINFRAME_H

#include <QFrame>
#include <QMessageBox>
#include <QString>
#include <QMouseEvent>

class MainWindow;

namespace Ui {
class LoginFrame;
}

class LoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LoginFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~LoginFrame();

signals:
    void loginSucceeded(const QString &id); // 로그인 성공 시그널

// 상단바 움직이기
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onLoginClicked(); // 버튼 처리

private:
    Ui::LoginFrame *ui;
    MainWindow* mainWindow;
    void setPlaceholders();
    void showLoginError();

private:
    QString m_id;
    QString m_pw;
    bool   m_drag    = false;
    QPoint m_dragPos;

};

#endif // LOGINFRAME_H
