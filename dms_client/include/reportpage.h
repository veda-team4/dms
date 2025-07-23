#ifndef REPORTPAGE_H
#define REPORTPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include "basepage.h"
#include "mainwindow.h"

namespace Ui {
class ReportPage;
}

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
    void readFrame();
};

#endif // REPORTPAGE_H
