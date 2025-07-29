#include "adddriverdialog.h"
#include "ui_adddriverdialog.h"

AddDriverDialog::AddDriverDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::AddDriverDialog)
{
    ui->setupUi(this);
}

AddDriverDialog::~AddDriverDialog()
{
    delete ui;
}
