#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~ConnectDialog();

    QString selectedIp();

private slots:
    void onConnectClicked();

private:
    Ui::ConnectDialog *ui;
    MainWindow* mainWindow;
    QString m_selectedIp;
};

#endif // CONNECTDIALOG_H
