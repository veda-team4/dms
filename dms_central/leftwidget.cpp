#include "leftwidget.h"
#include "ui_leftwidget.h"

LeftWidget::LeftWidget(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QWidget(parent)
    , ui(new Ui::LeftWidget)
{
    ui->setupUi(this);
}

LeftWidget::~LeftWidget()
{
    delete ui;
}
