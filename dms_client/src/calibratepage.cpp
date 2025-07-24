#include <QString>
#include "calibratepage.h"
#include "ui_calibratepage.h"
#include "utils.h"
#include "protocols.h"

CalibratePage::CalibratePage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) : BasePage(parent), mainWindow(mainWindow), socket(socket), ui(new Ui::CalibratePage) {
  ui->setupUi(this);
  ui->progressBar->setValue(0);
  connect(ui->nextButton, &QPushButton::clicked, this, &CalibratePage::moveToNextStep);
  connect(ui->previousButton, &QPushButton::clicked, this, &CalibratePage::moveToPreviousStep);

  finishTimer = new QTimer(this);
  finishTimer->setInterval(100);
  connect(finishTimer, &QTimer::timeout, this, [this]() {
    ++progressStep;
    int value = progressStep * 100 / progressTotalSteps;
    ui->progressBar->setValue(value);

    if (progressStep >= progressTotalSteps) {
      finishTimer->stop();
      moveToNextStep();
    }
    });
}

CalibratePage::~CalibratePage() {
  delete ui;
  delete finishTimer;
}

void CalibratePage::activate() {
  writeEncryptedCommand(socket, Protocol::CALIBRATE);
  connect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
  progressStep = 0;
  // ui->infoLabel->setText("뜬 눈의 크기를 측정합니다. 준비 완료 시 버튼을 눌러주세요.");
  ui->progressBar->setValue(0);
}

void CalibratePage::deactivate() {
  writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &CalibratePage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
}

void CalibratePage::moveToNextStep() {
  uint8_t type;
  uint32_t dataLen;
  double ear;
  switch (clickCount) {
  case 0:
    writeEncryptedCommand(socket, Protocol::CALIBRATE_OPENED);
    ui->nextButton->setEnabled(false);
    ui->previousButton->setEnabled(false);
    ui->infoLabel->setText("뜬 눈 크기 측정중. . . .");
    progressStep = 0;
    ui->progressBar->setValue(0);
    finishTimer->start();
    break;
  case 1:
    writeEncryptedCommand(socket, Protocol::CALIBRATE_FINISH);
    ui->nextButton->setEnabled(true);
    ui->previousButton->setEnabled(true);
    ui->infoLabel->setText("감은 눈의 크기를 측정합니다. 준비 완료 시 버튼을 눌러주세요.");
    break;
  case 2:
    writeEncryptedCommand(socket, Protocol::CALIBRATE_CLOSED);
    ui->nextButton->setEnabled(false);
    ui->previousButton->setEnabled(false);
    ui->infoLabel->setText("감은 눈 크기 측정중. . . .");
    progressStep = 0;
    ui->progressBar->setValue(0);
    finishTimer->start();
    break;
  case 3:
    writeEncryptedCommand(socket, Protocol::CALIBRATE_FINISH);
    ui->nextButton->setEnabled(true);
    ui->previousButton->setEnabled(true);
    ui->infoLabel->setText("눈 크기 측정 완료. 시작하려면 버튼을 눌러주세요.");
    break;
  case 4:
    emit moveToNext();
    return;
  }
  ++clickCount;
}

void CalibratePage::moveToPreviousStep() {
  switch (clickCount) {
  case 0:
    emit moveToPrevious();
    break;
  case 2:
    ui->infoLabel->setText("뜬 눈의 크기를 측정합니다. 준비 완료 시 버튼을 눌러주세요.");
    ui->progressBar->setValue(0);
    progressStep = 0;
    clickCount -= 2;
    break;
  case 4:
    ui->infoLabel->setText("감은 눈의 크기를 측정합니다. 준비 완료 시 버튼을 눌러주세요.");
    ui->progressBar->setValue(0);
    progressStep = 0;
    clickCount -= 2;
    break;
  }
}

void CalibratePage::readFrame() {
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
      else if (cmd == Protocol::OPENEDEAR || cmd == Protocol::CLOSEDEAR || cmd == Protocol::EARTHRESHOLD) {
        double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
        if (cmd == Protocol::OPENEDEAR) {
          //ui->openedVal->setText(QString::number(value));
        }
        else if (cmd == Protocol::CLOSEDEAR) {
          //ui->closedVal->setText(QString::number(value));
        }
        else if (cmd == Protocol::EARTHRESHOLD) {
          //ui->thresholdVal->setText(QString::number(value));
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
