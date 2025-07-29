#include "mainpage.h"
#include "ui_mainpage.h"
#include <QTimer>
#include <QVBoxLayout>

MainPage::MainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainPage)
{
    ui->setupUi(this);

    driver1 = new DriverPage(this);
    driver2 = new DriverPage(this);
    driver3 = new DriverPage(this);
    driver4 = new DriverPage(this);

    auto addDriverToFrame = [](QFrame *frame, DriverPage *driver) {
        QVBoxLayout *layout = new QVBoxLayout(frame);
        layout->setContentsMargins(0, 0, 0, 0); // border 가리지 않게
        layout->addWidget(driver);
    };

    addDriverToFrame(ui->frame_1, driver1);
    addDriverToFrame(ui->frame_2, driver2);
    addDriverToFrame(ui->frame_3, driver3);
    addDriverToFrame(ui->frame_4, driver4);
}

MainPage::~MainPage()
{
    delete ui;
    delete driver1;
    delete driver2;
    delete driver3;
    delete driver4;
}
