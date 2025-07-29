#ifndef DRIVERPAGE_H
#define DRIVERPAGE_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

namespace Ui {
class DriverPage;
}

class DriverPage : public QWidget
{
    Q_OBJECT

public:
    explicit DriverPage(QWidget *parent = nullptr);
    ~DriverPage();

private slots:
    void handleButtonClick();
    void updateFrame();

private:
    Ui::DriverPage *ui;
    QTimer* frameTimer;
    //cv::VideoCapture cap;
    bool connected;
};

#endif // DRIVERPAGE_H
