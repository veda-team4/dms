#ifndef CENTERWIDGET_H
#define CENTERWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
class CenterWidget;
}

class CenterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CenterWidget(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~CenterWidget();

private:
    Ui::CenterWidget *ui;
    MainWindow* mainWindow;
};

#endif // CENTERWIDGET_H
