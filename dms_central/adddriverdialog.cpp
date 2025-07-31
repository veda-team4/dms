#include "adddriverdialog.h"
#include "ui_adddriverdialog.h"
#include <QString>
#include <QPushButton>

AddDriverDialog::AddDriverDialog(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QDialog(parent)
    , ui(new Ui::AddDriverDialog)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &AddDriverDialog::onConfirmClicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &AddDriverDialog::reject);
}

AddDriverDialog::~AddDriverDialog()
{
    delete ui;
}

CameraInfo AddDriverDialog::cameraInfo() const {
    return m_info;
}

void AddDriverDialog::onConfirmClicked() {
    m_info.name = ui->lineEdit->text();
    m_info.ip   = ui->lineEdit_2->text();
    accept();
}
