#include "centerwidget.h"
#include "ui_centerwidget.h"

CenterWidget::CenterWidget(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QWidget(parent)
    , ui(new Ui::CenterWidget)
{
    ui->setupUi(this);
}

CenterWidget::~CenterWidget()
{
    delete ui;
}
