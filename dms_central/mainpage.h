#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QLabel>
#include "driverpage.h"

namespace Ui {
class MainPage;
}

class MainPage : public QWidget
{
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage();

private:
    Ui::MainPage *ui;

    DriverPage* driver1, * driver2, * driver3, * driver4;
};

#endif // MAINPAGE_H
