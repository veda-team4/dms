#ifndef DELETEDRIVERDIALOG_H
#define DELETEDRIVERDIALOG_H

#include <QDialog>
#include <Camerainfo.h>

class MainWindow;

namespace Ui {
class DeleteDriverDialog;
}

class DeleteDriverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteDriverDialog(const CameraInfo& info, MainWindow* mainWindow, QWidget *parent = nullptr);
    ~DeleteDriverDialog();

private:
    Ui::DeleteDriverDialog *ui;
    MainWindow* mainWindow;
};

#endif // DELETEDRIVERDIALOG_H
