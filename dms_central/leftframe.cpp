#include "leftframe.h"
#include "ui_leftframe.h"

LeftFrame::LeftFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::LeftFrame)
{
    ui->setupUi(this);
}

LeftFrame::~LeftFrame()
{
    delete ui;
}
