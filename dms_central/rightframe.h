#ifndef RIGHTFRAME_H
#define RIGHTFRAME_H

#include <QFrame>

class MainWindow;

namespace Ui {
class RightFrame;
}

class RightFrame : public QFrame
{
    Q_OBJECT

public:
    explicit RightFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~RightFrame();

private:
    Ui::RightFrame *ui;
    MainWindow* mainWindow;
};

#endif // RIGHTFRAME_H
