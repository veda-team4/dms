#include "monitorpage.h"
#include "ui_monitorpage.h"
#include "utils.h"
#include "protocols.h"
#include <unistd.h>
#include <QDateTime>
#include <iomanip>
#include <sstream>
#include <string>

#define DEVICE_ON 0

MonitorPage::MonitorPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) : BasePage(parent), mainWindow(mainWindow), ui(new Ui::MonitorPage), socket(socket) {
  ui->setupUi(this);
  connect(ui->previousButton, &QPushButton::clicked, this, &MonitorPage::moveToPrevious);
  connect(ui->nextButton, &QPushButton::clicked, this, &MonitorPage::moveToNext);

  prevHeadDowntime = std::chrono::steady_clock::now();

  ui->naviWidget->hide();
  speaker = new Speaker("plughw:4,0");
#if DEVICE_ONf
  openDevice(); 
  int i;
  for (i = 0; i < 100; ++i) {
    gps->cur_location(&latitude, &longitude);
    if (33.0 <= latitude && latitude <= 43) {
      writeLog("Gps connected");
      break;
    }
    usleep(500000);
  }
  if (i == 100) {
    writeLog("Gps not connected");
  }
  co2_v = 0;
  co2Timer = new QTimer(this);
  gpsTimer = new QTimer(this);
  connect(co2Timer, &QTimer::timeout, this, [this]() {
      co2->read_CTH(&co2_v, &temp, &hum);
      ui->co2value->setText(QString::fromStdString(std::to_string(co2_v)));
      if(co2_v >= 1200) {
        ui->co2alarmtext->setVisible(true);
      }
      else {
        ui->co2alarmtext->setVisible(false);
      }

      if(co2_v < 1000) {
          ui->co2Safe->setVisible(true);
          ui->co2Warning->setVisible(false);
          ui->co2Danger->setVisible(false);
      }
      else if(co2_v < 1200) {
          ui->co2Safe->setVisible(false);
          ui->co2Warning->setVisible(true);
          ui->co2Danger->setVisible(false);
      }
      else {
          ui->co2Safe->setVisible(false);
          ui->co2Warning->setVisible(false);
          ui->co2Danger->setVisible(true);
      }

  });
  connect(gpsTimer, &QTimer::timeout, this, [this]() {
      double lat, lon;
      gps->cur_location(&lat, &lon);
      if (33.0 <= lat && lat <= 43.0) {
        totalKm += osrm->getDistance(latitude, longitude, lat, lon);
        latitude = lat;
        longitude = lon;
      }
  });
#endif
  wakeupTimer = new QTimer(this);
  connect(wakeupTimer, &QTimer::timeout, this, [this]() {
    speaker->play("wakeup.wav");
  });
}

MonitorPage::~MonitorPage()
{
    delete ui;
    delete speaker;
#if DEVICE_ON
    delete co2Timer;
    delete gpsTimer;
    delete wakeupTimer;
    closeDevice();
#endif
}

void MonitorPage::openDevice() {
  led = new Led();
  gps = new Gps();
  co2 = new CO2Sensor();
  bluetooth = new Bluetooth();
  osrm = new Osrm();
}

void MonitorPage::closeDevice() {
  led->led_off();
  delete led;
  delete gps;
  delete bluetooth;
  delete osrm;
}

void MonitorPage::wakeupUI(bool on) {
  if (on && !wakeupFlashing) {
    wakeupFlashing = true;
    ++mainWindow->info.alertCount;
    ui->background_red->setVisible(true);
    ui->highWarning->setVisible(true);
    ui->infotext->setVisible(false);
    ui->infoalarm->raise();
    wakeupTimer->start(1000);
 #if DEVICE_ON
    led->led_on();
    bluetooth->Motor();
 #endif
    navigation(true);

    // 최근 5초 프레임을 클립으로 저장
    std::vector<QPixmap> clip(mainWindow->recentFrames.begin(), mainWindow->recentFrames.end());
    mainWindow->sleepingFrames.push_back(std::move(clip));

    // 졸음 발생 시간 저장
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local = std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(local, "%H:%M");
    mainWindow->sleepingTimes.push_back(oss.str());
  }
  else if (!on && wakeupFlashing) {
    wakeupFlashing = false;
    ui->background_red->setVisible(false);
    ui->highWarning->setVisible(false);
    ui->infotext->setVisible(true);
    if (mainWindow->isLock()) {
      ui->infojes->raise();
    }
    else {
      ui->infoswipe->raise();
    }
#if DEVICE_ON
    led->led_off();
    bluetooth->Motor();
#endif
    wakeupTimer->stop();
    navigation(false);
  }
}

void MonitorPage::navigation(bool on) {
#if DEVICE_ON
    if (on) {
      ui->naviWidget->setVisible(true);
      #if DEVICE_ON
      gps->cur_location(&latitude, &longitude);

      restArea area = osrm->getRestAreas(latitude, longitude);
      ui->toLabel->setText((area.isRestArea ? QString("휴게소") : QString("졸음쉼터")));
      ui->restNameLabel->setText(QString::fromStdString(area.name) + (area.isRestArea ? QString("휴게소") : QString("졸음쉼터")));
      std::string dist = std::to_string(area.route_distance / 1000);
      size_t dot = dist.find('.');
      if (dot != std::string::npos && dot + 3 < dist.length()) {
      dist = dist.substr(0, dot + 3);
      }
      ui->kmLabel->setText(QString::fromStdString(dist + std::string(" KM")));
      ui->timeLabel->setText(QString::fromStdString(std::to_string((int)(area.route_duration / 60)) + std::string(" 분 소요 예정")));
      #endif
    }
    else {
      ui->naviWidget->setVisible(false);
    }
#endif
}

void MonitorPage::activate() {
  mainWindow->startTime = std::chrono::steady_clock::now();
  connect(socket, &QLocalSocket::readyRead, this, &MonitorPage::readFrame);
  writeEncryptedCommand(socket, Protocol::MONITOR);
  mainWindow->info.clear();
  if(!mainWindow->isLock()) {
      mainWindow->updateLock();
      writeEncryptedCommand(socket, Protocol::LOCK);
  }
  totalKm = 0.0;
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
  ui->infojes->raise();
  ui->msgFrame->hide();
#if DEVICE_ON
  gps->cur_location(&latitude, &longitude);
  
  co2Timer->start(5000);
  gpsTimer->start(60000);
#endif
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
  mainWindow->info.totalDistance = totalKm;
#if DEVICE_ON
  co2Timer->stop();
  gpsTimer->stop();
#endif
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
        auto now_t = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now_t - prevHeadDowntime).count() <= 10) {
          return;
        }
        else {
          prevHeadDowntime = now_t;
        }
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
          if (mainWindow->isLock()) {
            ui->infojes->raise();
          }
          else {
            ui->infoswipe->raise();
          }
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

        if (v < 40) {
            ui->sleepingBar->setStyleSheet("QProgressBar { border: 2px solid #66cc66; border-radius: 5px; background-color: #0e1420; outline: none; color: white; } QProgressBar:chunk { border-radius: 3px; background-color: #66cc66; }");
        }
        else if (v < 80) {
            ui->sleepingBar->setStyleSheet("QProgressBar { border: 2px solid #FFB400; border-radius: 5px; background-color: #0e1420; outline: none; color: white; } QProgressBar:chunk { border-radius: 3px; background-color: #FFB400; }");
        }
        else {
            ui->sleepingBar->setStyleSheet("QProgressBar { border: 2px solid #FF4444; border-radius: 5px; background-color: #0e1420; outline: none; color: white; } QProgressBar:chunk { border-radius: 3px; background-color: #FF4444; }");
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
      else if (cmd == Protocol::MESSAGE) {
        // ---- MESSAGE 처리 추가 ----
        QByteArray messageData = QByteArray::fromRawData(decrypted.constData() + 5, dataLen);

        QString message = QString::fromUtf8(messageData);

        // 메시지 알림 UI 띄우기
        ui->msgLabel->setText(message);
        ui->msgFrame->show();
        ui->msgFrame->raise();
        speaker->play("message.wav");
        QTimer::singleShot(5000, this, [=]() {
            ui->msgFrame->hide();
        });
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
