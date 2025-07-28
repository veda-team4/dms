#include "centerframe.h"
#include "ui_centerframe.h"

CenterFrame::CenterFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::CenterFrame)
{
    ui->setupUi(this);
}

CenterFrame::~CenterFrame()
{
    delete ui;
}
