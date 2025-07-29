#include "centerframe.h"
#include "ui_centerframe.h"

CenterFrame::CenterFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::CenterFrame)
{
    ui->setupUi(this);

    cam0 = new CamFrame(mainWindow, this);
    cam0->move(16, 35);
    cam1 = new CamFrame(mainWindow, this);
    cam1->move(356, 35);
    cam2 = new CamFrame(mainWindow, this);
    cam2->move(16, 315);
    cam3 = new CamFrame(mainWindow, this);
    cam3->move(356, 315);
}

CenterFrame::~CenterFrame()
{
    delete ui;
    delete cam0;
    delete cam1;
    delete cam2;
    delete cam3;
}
