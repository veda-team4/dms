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

private:
    Ui::AddDriverDialog *ui;
    MainWindow* mainWindow;
};

#endif // ADDDRIVERDIALOG_H
