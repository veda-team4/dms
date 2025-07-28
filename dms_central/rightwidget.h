#ifndef RIGHTWIDGET_H
#define RIGHTWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
class RightWidget;
}

class RightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RightWidget(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~RightWidget();

private:
    Ui::RightWidget *ui;
    MainWindow* mainWindow;
};

#endif // RIGHTWIDGET_H
