#ifndef TOPFRAME_H
#define TOPFRAME_H

#include <QFrame>

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

private:
    Ui::TopFrame *ui;
    MainWindow* mainWindow;
};

#endif // TOPFRAME_H
