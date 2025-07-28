#include "rightwidget.h"
#include "ui_rightwidget.h"

RightWidget::RightWidget(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QWidget(parent)
    , ui(new Ui::RightWidget)
{
    ui->setupUi(this);
}

RightWidget::~RightWidget()
{
    delete ui;
}
