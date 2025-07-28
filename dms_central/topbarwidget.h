#ifndef TOPBARWIDGET_H
#define TOPBARWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
class TopBarWidget;
}

class TopBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopBarWidget(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~TopBarWidget();

private:
    Ui::TopBarWidget *ui;
    MainWindow* mainWindow;
};

#endif // TOPBARWIDGET_H
