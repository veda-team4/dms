#ifndef LEFTWIDGET_H
#define LEFTWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
class LeftWidget;
}

class LeftWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LeftWidget(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~LeftWidget();

private:
    Ui::LeftWidget *ui;
    MainWindow* mainWindow;
};

#endif // LEFTWIDGET_H
