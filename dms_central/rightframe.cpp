#include "rightframe.h"
#include "ui_rightframe.h"

RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);
}

RightFrame::~RightFrame()
{
    delete ui;
}
