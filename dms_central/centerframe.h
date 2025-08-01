#ifndef CENTERFRAME_H
#define CENTERFRAME_H

#include <QFrame>
#include "camframe.h"

class MainWindow;

namespace Ui {
class CenterFrame;
}

class CenterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CenterFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~CenterFrame();

    CamFrame* cam0, * cam1, * cam2, * cam3;

private:
    Ui::CenterFrame *ui;
    MainWindow* mainWindow;
};

#endif // CENTERFRAME_H
