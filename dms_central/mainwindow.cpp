#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loginPage = new LoginPage();
    mainPage = new MainPage();
    ui->stackedWidget->addWidget(loginPage);
    ui->stackedWidget->addWidget(mainPage);
    ui->stackedWidget->setCurrentWidget(loginPage);

    connect(loginPage, &LoginPage::loginSuccess, this, &MainWindow::showMainPage);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete loginPage;
    delete mainPage;
}

void MainWindow::showMainPage() {
    ui->stackedWidget->setCurrentWidget(mainPage);
}
