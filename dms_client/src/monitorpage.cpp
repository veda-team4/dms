#include "monitorpage.h"
#include "ui_monitorpage.h"
#include "utils.h"
#include "protocols.h"
#include <unistd.h>

MonitorPage::MonitorPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) : BasePage(parent), mainWindow(mainWindow), ui(new Ui::MonitorPage), socket(socket) {
  ui->setupUi(this);
  connect(ui->previousButton, &QPushButton::clicked, this, &MonitorPage::moveToPrevious);

  // WakeUP 레이블 번쩍번쩍 기능
  wakeupTimer = new QTimer(this);
  connect(wakeupTimer, &QTimer::timeout, this, [=]() {
    if (wakeupFlashOn) {
      ui->wakeupLabel->setStyleSheet("background-color: white; color: black; font-size: 24px; font-weight: bold; border-radius: 8px; border: 2px solid black;");
    }
    else {
      ui->wakeupLabel->setStyleSheet("background-color: red; color: black; font-size: 24px; font-weight: bold; border-radius: 8px; border: 2px solid black;");
    }
    wakeupFlashOn = !wakeupFlashOn;
    });

  ui->wakeupLabel->hide();
  ui->wakeupCloseButton->hide();

  connect(ui->wakeupCloseButton, &QPushButton::clicked, this, [=]() {
    wakeupTimer->stop();
    ui->wakeupLabel->hide();
    ui->wakeupCloseButton->hide();
    wakeupFlashing = false;
    });

  ui->naviWidget->hide();

  // led = new Led();
  // speaker = new Speaker("plughw:3,0");
  // gps = new Gps();
  // osrm = new Osrm();
}

MonitorPage::~MonitorPage()
{
  // led->led_off();
  // delete led;
  // delete speaker;
  // delete gps;
  // delete osrm;
  delete wakeupTimer;
  delete ui;
}

void MonitorPage::wakeupUI(bool on) {
  if (on && !wakeupFlashing) {
    wakeupFlashing = true;
    ui->wakeupLabel->show();
    ui->wakeupCloseButton->show();
    wakeupTimer->start(300);
    // led->led_on();
    // speaker->play("tts.wav");
  }
  else if (!on && wakeupFlashing) {
    wakeupFlashing = false;
    wakeupTimer->stop();
    ui->wakeupLabel->hide();
    ui->wakeupCloseButton->hide();
    // led->led_off();
    navigation(true);
  }
}

void MonitorPage::navigation(bool on) {
  /*
    if (on && !navigating) {
        while (!gps->cur_location(&latitude, &longitude)) {
          usleep(100);
        }
        restArea area = osrm->getRestAreas(latitude, longitude);
        ui->restNameLabel->setText(QString::fromStdString(area.name) + (area.isRestArea ? QString(" 휴게소") : QString(" 졸음쉼터")));
        std::string dist = std::to_string(area.route_distance / 1000);
        size_t dot = dist.find('.');
        if (dot != std::string::npos && dot + 3 < dist.length()) {
        dist = dist.substr(0, dot + 3);
        }
        ui->kmLabel->setText(QString::fromStdString(dist + std::string(" KM")));
        ui->timeLabel->setText(QString::fromStdString(std::to_string((int)(area.route_duration / 60)) + std::string(" 분")));
        ui->naviWidget->show();
        navigating = true;
        writeLog(std::string("latitude: ") + std::to_string(latitude) + std::string(", longitude: ") + std::to_string(longitude));
    }
    else if (!on && navigating) {
        ui->naviWidget->hide();
        navigating = false;
    }
  */
}

void MonitorPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  writeEncryptedCommand(socket, Protocol::MONITOR);
}

void MonitorPage::deactivate() {
  writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
  wakeupUI(false);
}

void MonitorPage::readFrame() {
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

      if (cmd == Protocol::HEADDROPPED) {
        wakeupUI(true);
        return;
      }

      if (cmd == Protocol::LEFT) {
        if (!mainWindow->isLock()) {
          ui->previousButton->click();
        }
        return;
      }
      else if (cmd == Protocol::RIGHT) {
        return;
      }
      else if (cmd == Protocol::STRETCH) {
        if (navigating) {
            navigation(false);
            return;
        }
        else if (wakeupFlashing) {
          wakeupUI(false);
          return;
        }
        else {
          mainWindow->updateLock();
          writeEncryptedCommand(socket, (mainWindow->isLock() ? Protocol::LOCK : Protocol::UNLOCK));
          return;
        }
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
      else if (cmd == Protocol::EYECLOSEDRATIO) {
        double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
        ui->sleepingBar->setValue((int)(value * 100.0));

        if (value >= BLINK_RATIO_THRESH) {
          if (navigating) {
            navigation(false);
          }
          wakeupUI(true);
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
