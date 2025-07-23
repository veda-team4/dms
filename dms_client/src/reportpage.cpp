#include "reportpage.h"
#include "ui_reportpage.h"
#include "protocols.h"
#include "utils.h"

ReportPage::ReportPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) :
    BasePage(parent), mainWindow(mainWindow), socket(socket),
    ui(new Ui::ReportPage)
{
    ui->setupUi(this);
}

ReportPage::~ReportPage()
{
    delete ui;
}

void ReportPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &ReportPage::readFrame);
  writeEncryptedCommand(socket, Protocol::REPORT);
}

void ReportPage::deactivate() {
  // writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &ReportPage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
}

void ReportPage::readFrame() {

}
