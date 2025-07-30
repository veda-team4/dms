#ifndef RIGHTFRAME_H
#define RIGHTFRAME_H

#include <QFrame>

#include <QScrollArea>
#include <QLabel>
#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QVector>

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

    //QWidget *contentWidget;
    QScrollArea *scrollArea;
    int yOffset;
    QVector<QWidget*> eventWidgets;

    void alarmPage();
    void eventPage();
    void addNewWidget();
    void customWidget();
};

#endif // RIGHTFRAME_H
