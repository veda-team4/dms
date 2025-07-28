#ifndef CENTERFRAME_H
#define CENTERFRAME_H

#include <QFrame>

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

private:
    Ui::CenterFrame *ui;
    MainWindow* mainWindow;
};

#endif // CENTERFRAME_H
