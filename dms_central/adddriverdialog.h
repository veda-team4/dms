#ifndef ADDDRIVERDIALOG_H
#define ADDDRIVERDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class AddDriverDialog;
}

class AddDriverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDriverDialog(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~AddDriverDialog();

    QString dcamName();

private slots:
    void getdcamName();

private:
    Ui::AddDriverDialog *ui;
    MainWindow* mainWindow;
    QString m_dcamName;
};

#endif // ADDDRIVERDIALOG_H
