#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QLabel>
#include <opencv2/opencv.hpp>

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
    QVector<cv::VideoCapture*> captures;
    QVector<QLabel*> videoLabels;
    QTimer* frameTimer;

private slots:
    void openConnectDialog();
    void addStream(const QString& url);
    void updateFrames();
};

#endif // MAINPAGE_H
