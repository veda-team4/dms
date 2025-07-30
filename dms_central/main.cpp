#include "mainwindow.h"
#include "loginframe.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // "Login message box" custom design sheet 적용
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);

    QApplication a(argc, argv);
    MainWindow w;
    LoginFrame login(&w);

    // Set ID
    QObject::connect(&login, &LoginFrame::loginSucceeded, &w, &MainWindow::setId);
    // Show MainWindow
    QObject::connect(&login, &LoginFrame::loginSucceeded, &w, &MainWindow::show);

    login.show();
    return a.exec();

}
