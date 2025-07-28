#include "topframe.h"
#include "ui_topframe.h"

TopFrame::TopFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::TopFrame)
{
    ui->setupUi(this);
}

TopFrame::~TopFrame()
{
    delete ui;
}
