#include <QPixmap>
#include "camsetpage.h"
#include "ui_camsetpage.h"
#include "utils.h"
#include "protocols.h"

CamSetPage::CamSetPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) : BasePage(parent), mainWindow(mainWindow), socket(socket), ui(new Ui::CamSetPage) {
  ui->setupUi(this);
  connect(ui->nextButton, &QPushButton::clicked, this, &CamSetPage::moveToNext);
  connect(ui->previousButton, &QPushButton::clicked, this, &CamSetPage::moveToPrevious);
}

CamSetPage::~CamSetPage() {
  delete ui;
}

void CamSetPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readSocket);
  writeEncryptedCommand(socket, Protocol::CAMSET);
}

void CamSetPage::deactivate() {
  writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &CamSetPage::readSocket);
  while (socket->waitForReadyRead(100)) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
}

void CamSetPage::readSocket() {
  buffer.append(socket->readAll());

  while (true) {
    // 단계 1: IV + 길이 수신 대기
    if (ciphertext_len == -1 && buffer.size() >= 20) {
      // 16바이트 IV 읽기
      iv = buffer.left(16);
      buffer.remove(0, 16);

      // 4바이트 암호문 길이 읽기
      ciphertext_len = *reinterpret_cast<const uint32_t*>(buffer.constData());
      buffer.remove(0, 4);
    }

    // 데이터 길이만큼 수신 완료되었을 때 처리
    if (ciphertext_len != -1 && buffer.size() >= ciphertext_len) {
      QByteArray encrypted = buffer.left(ciphertext_len);
      buffer.remove(0, ciphertext_len);
      ciphertext_len = -1;

      // 복호화
      QByteArray decrypted;
      decrypted.resize(131072);
      int decrypted_len;

      bool success = aes_decrypt(
        reinterpret_cast<const unsigned char*>(encrypted.constData()), encrypted.size(),
        key, reinterpret_cast<const unsigned char*>(iv.constData()),
        reinterpret_cast<unsigned char*>(decrypted.data()), &decrypted_len
      );

      if (!success) {
        writeLog("AES decrypt failed");
        return;
      }

      // 복호화된 평문에서 명령과 길이 추출
      quint8 cmd = static_cast<quint8>(decrypted[0]);

      if (cmd == Protocol::RIGHT) {
        if (!mainWindow->isLock()) {
          ui->nextButton->click();
        }
        return;
      }
      else if (cmd == Protocol::LEFT) {
        if (!mainWindow->isLock()) {
          ui->previousButton->click();
        }
        return;
      }
      else if (cmd == Protocol::STRETCH) {
        mainWindow->updateLock();
        writeEncryptedCommand(socket, (mainWindow->isLock() ? Protocol::LOCK : Protocol::UNLOCK));
        if(mainWindow->isLock()) {
            ui->infojes->raise();
        }
        else {
            ui->infoswipe->raise();
        }
        return;
      }
      else if (cmd == Protocol::FACEOK) {
        ui->camset_suc->raise();
        return;
      }
      else if (cmd == Protocol::FACENOTOK) {
        ui->camset_fail->raise();
        return;
      }

      quint32 dataLen = *reinterpret_cast<const quint32*>(decrypted.constData() + 1);

      if (cmd == Protocol::FRAME) {
        QByteArray imageData = QByteArray::fromRawData(decrypted.constData() + 5, dataLen);

        QPixmap pixmap;
        if (pixmap.loadFromData(imageData, "JPG")) {
          ui->videoLabel->setPixmap(
            pixmap.scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)
          );
        }
      }
      else {
        writeLog("Clear protocol number " + std::to_string(cmd));
      }
    }
    else {
      break;  // 아직 데이터 부족
    }
  }
}
