#include "topbarwidget.h"
#include "ui_topbarwidget.h"

TopBarWidget::TopBarWidget(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QWidget(parent)
    , ui(new Ui::TopBarWidget)
{
    ui->setupUi(this);
}

TopBarWidget::~TopBarWidget()
{
    delete ui;
}
