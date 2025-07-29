#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 상단, 좌측, 중앙, 우측 위젯 선언
    topFrame = new TopFrame(this, ui->centralwidget);
    topFrame->move(0, 0);
    leftFrame = new LeftFrame(this, ui->centralwidget);
    leftFrame->move(0, 40);
    centerFrame = new CenterFrame(this, ui->centralwidget);
    centerFrame->move(200, 40);
    rightFrame = new RightFrame(this, ui->centralwidget);
    rightFrame->move(900, 40);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete topFrame;
    delete leftFrame;
    delete centerFrame;
    delete rightFrame;
}
