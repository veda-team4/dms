#ifndef ADDDRIVERDIALOG_H
#define ADDDRIVERDIALOG_H

#include <QDialog>
#include <Camerainfo.h>

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

    CameraInfo cameraInfo() const;

private slots:
    void onConfirmClicked();

private:
    Ui::AddDriverDialog *ui;
    MainWindow* mainWindow;
    CameraInfo m_info;
};

#endif // ADDDRIVERDIALOG_H
