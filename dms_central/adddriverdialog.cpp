#include "adddriverdialog.h"
#include "ui_adddriverdialog.h"
#include <QString>
#include <QPushButton>

AddDriverDialog::AddDriverDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::AddDriverDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &AddDriverDialog::getdcamName);
}

AddDriverDialog::~AddDriverDialog()
{
    delete ui;
}

QString AddDriverDialog::dcamName() {
    return m_dcamName;
}

void AddDriverDialog::getdcamName(){
    m_dcamName = ui->lineEdit->text();
    accept();
}
