#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "topbarwidget.h"
#include "leftwidget.h"
#include "centerwidget.h"
#include "rightwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    TopBarWidget* topBarWidget;
    LeftWidget* leftWidget;
    CenterWidget* centerWidget;
    RightWidget* rightWidget;

private slots:
};
#endif // MAINWINDOW_H
