#ifndef REPORTPAGE_H
#define REPORTPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QTimer>
#include "basepage.h"
#include "mainwindow.h"

namespace Ui {
class ReportPage;
}

struct Information;

class ReportPage : public BasePage
{
    Q_OBJECT

public:
    explicit ReportPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket);
    ~ReportPage();
    void activate() override;
    void deactivate() override;

private:
    Ui::ReportPage *ui;
    MainWindow* mainWindow;
    QLocalSocket* socket;
    QByteArray buffer;
    QByteArray iv;
    int ciphertext_len = -1;
    quint8 cmd;
    int currentFrameIndex;
    QTimer* timer = nullptr;
    void readFrame();
    void printGraph(Information& info);
    void printSummary(Information& info);
    void playSleepingClip();
};

#endif // REPORTPAGE_H
