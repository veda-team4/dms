#include "mainwindow.h"
#include "loginframe.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginFrame login;
    MainWindow w;

    QObject::connect(&login, &LoginFrame::loginSucceeded, &w, &MainWindow::show);

    login.show();

    return a.exec();

}
