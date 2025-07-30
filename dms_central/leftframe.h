#ifndef LEFTFRAME_H
#define LEFTFRAME_H

#include <QFrame>

class MainWindow;

namespace Ui {
class LeftFrame;
}

class LeftFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LeftFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~LeftFrame();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::LeftFrame *ui;
    MainWindow* mainWindow;
};

#endif // LEFTFRAME_H
