#include "driverpage.h"
#include "ui_driverpage.h"
#include "connectdialog.h"

DriverPage::DriverPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DriverPage)
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, &DriverPage::handleButtonClick);

    frameTimer = new QTimer(this);
    connect(frameTimer, &QTimer::timeout, this, &DriverPage::updateFrame);
}

DriverPage::~DriverPage()
{
    delete ui;
}

void DriverPage::handleButtonClick() {
}

void DriverPage::updateFrame() {
}

