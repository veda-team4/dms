#ifndef TOPFRAME_H
#define TOPFRAME_H

#include <QFrame>
#include <QTimer>
#include <QTime>

class MainWindow;

namespace Ui {
class TopFrame;
}

class TopFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TopFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~TopFrame();
    void setAdminID(const QString &id); // userID 설정
    void startClock(); // 시계 시작

private slots:
    void updateTime();

private:
    Ui::TopFrame *ui;
    MainWindow* mainWindow;
    QTimer *clockTimer;
};

#endif // TOPFRAME_H
