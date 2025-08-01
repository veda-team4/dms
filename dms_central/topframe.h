#ifndef TOPFRAME_H
#define TOPFRAME_H

#include <QFrame>
#include <QTimer>
#include <QTime>
#include <QMouseEvent>

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

// 상단바 움직이기
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateTime();

private:
    Ui::TopFrame *ui;
    MainWindow* mainWindow;
    QTimer *clockTimer;

    bool   m_drag    = false;
    QPoint m_dragPos;
};

#endif // TOPFRAME_H
