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
    // -----
    CustomScrollArea(QWidget* parent = nullptr) : QScrollArea(parent), yOffset(0) {
        container = new QWidget;
        container->setMinimumHeight(1);  // 초기 높이
        container->setMinimumWidth(300); // 너비는 고정 가능

        setWidget(container);
        setWidgetResizable(true);
    }

    void addNewWidget(QWidget* widget) {
        widget->setParent(container);
        widget->move(0, yOffset);
        widget->show();

        yOffset += widget->height();  // 다음 위젯 위치 계산
        container->resize(container->width(), yOffset);  // 높이 갱신

        // 스크롤을 맨 아래로 이동
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }

private:
    QWidget* container;
    int yOffset;
    // -----

private:
    Ui::RightFrame *ui;
    MainWindow* mainWindow;


    QWidget *contentWidget;
    QScrollArea *scrollArea;
    int count = 0;

private slots:
    void addNewWidget();
};

#endif // RIGHTFRAME_H
