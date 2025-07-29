#include "camframe.h"
#include "ui_camframe.h"

CamFrame::CamFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::CamFrame)
{
    ui->setupUi(this);
}

CamFrame::~CamFrame()
{
    delete ui;
}
