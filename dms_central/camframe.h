#ifndef CAMFRAME_H
#define CAMFRAME_H

#include <QFrame>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMovie>

class MainWindow;

namespace Ui {
class CamFrame;
}

class CamFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CamFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~CamFrame();

private:
    Ui::CamFrame *ui;
    MainWindow* mainWindow;
    QMediaPlayer* player = nullptr;
    QVideoWidget* videoWidget = nullptr;
    QMovie* loadingGif;
    void connectRtsp(QString ip);
    void disconnectRtsp();
};

#endif // CAMFRAME_H
