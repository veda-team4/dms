#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 상단, 좌측, 중앙, 우측 위젯 선언
    topBarWidget = new TopBarWidget(this, ui->frameTopBar);
    topBarWidget->setGeometry(0, 0, ui->frameTopBar->width(), ui->frameTopBar->height());
    leftWidget = new LeftWidget(this, ui->frameLeft);
    leftWidget->setGeometry(0, 0, ui->frameLeft->width(), ui->frameLeft->height());
    centerWidget = new CenterWidget(this, ui->frameCenter);
    centerWidget->setGeometry(0, 0, ui->frameCenter->width(), ui->frameCenter->height());
    rightWidget = new RightWidget(this, ui->frameRight);
    rightWidget->setGeometry(0, 0, ui->frameRight->width(), ui->frameRight->height());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete topBarWidget;
    delete leftWidget;
    delete centerWidget;
    delete rightWidget;
}
