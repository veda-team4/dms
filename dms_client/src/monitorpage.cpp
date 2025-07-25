#include "monitorpage.h"
#include "ui_monitorpage.h"
#include "utils.h"
#include "protocols.h"
#include <unistd.h>
#include <QDateTime>

MonitorPage::MonitorPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) : BasePage(parent), mainWindow(mainWindow), ui(new Ui::MonitorPage), socket(socket) {
  ui->setupUi(this);
  connect(ui->previousButton, &QPushButton::clicked, this, &MonitorPage::moveToPrevious);
  connect(ui->nextButton, &QPushButton::clicked, this, &MonitorPage::moveToNext);

  ui->naviWidget->hide();
}

MonitorPage::~MonitorPage()
{
    delete ui;
}

void MonitorPage::openDevice() {
  /*
  led = new Led();
  speaker = new Speaker("plughw:4,0");
  gps = new Gps();
  bluetooth = new Bluetooth();
  osrm = new Osrm();
  */
}

void MonitorPage::closeDevice() {
  /*
  led->led_off();
  delete led;
  delete speaker;
  delete gps;
  delete bluetooth;
  delete osrm;
  */
}

void MonitorPage::playDevice() {
    /*
    led->led_on();
    speaker->play("tts.wav");
    navigation(true);
    */
}

void MonitorPage::stopDevice() {
    /*
    led->led_off();
    navigation(false);
    */
}

void MonitorPage::wakeupUI(bool on) {
  if (on && !wakeupFlashing) {
    wakeupFlashing = true;
    ++mainWindow->info.alertCount;
    ui->background_red->setVisible(true);
    ui->highWarning->setVisible(true);
    playDevice();

    // 최근 5초 프레임을 클립으로 저장
    std::vector<QPixmap> clip(mainWindow->recentFrames.begin(), mainWindow->recentFrames.end());
    mainWindow->sleepingFrames.push_back(std::move(clip));
  }
  else if (!on && wakeupFlashing) {
    wakeupFlashing = false;
    ui->background_red->setVisible(false);
    ui->highWarning->setVisible(false);
    stopDevice();
  }
}

void MonitorPage::navigation(bool on) {
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
}

void MonitorPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  writeEncryptedCommand(socket, Protocol::MONITOR);
  mainWindow->info.clear();
  if(!mainWindow->isLock()) {
      mainWindow->updateLock();
      writeEncryptedCommand(socket, Protocol::LOCK);
  }
  openDevice();
  ui->background_red->setVisible(false);
  ui->co2alarmtext->setVisible(false);
  ui->middleWarning->setVisible(false);
  ui->highWarning->setVisible(false);
  ui->co2Warning->setVisible(false);
  ui->co2Danger->setVisible(false);
  ui->co2Safe->setVisible(false);
  ui->dangerface->setVisible(false);
  ui->warningface->setVisible(false);
  ui->safeface->setVisible(true);
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
  closeDevice();
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
        if (!mainWindow->isLock()) {
          ui->nextButton->click();
        }
        return;
      }
      else if (cmd == Protocol::STRETCH) {
        if (wakeupFlashing) {
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

        mainWindow->recentFrames.push_back(pixmap);
        if(mainWindow->recentFrames.size() > mainWindow->MAX_FRAMES) {
          mainWindow->recentFrames.pop_front();
        }
      }
      else if (cmd == Protocol::EYECLOSEDRATIO) {
        double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
        int v = (int)(value * 100.0);
        ui->sleepingBar->setValue(v);
        ui->sleepingProgress->setValue(v);

        if (value == 1.0) {
          wakeupUI(true);
        }

        // ---- 1초마다 values에 추가 ----
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - lastAppendTime >= 1000 || (!mainWindow->info.values.empty() && mainWindow->info.values.back() != 100 && value == 1.0)) {
            mainWindow->info.values.append(v);
            lastAppendTime = now;
        }
        ++mainWindow->info.sleepingCount;
        mainWindow->info.sleepingAverage += (v - mainWindow->info.sleepingAverage) / mainWindow->info.sleepingCount;
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
