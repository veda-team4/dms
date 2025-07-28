#ifndef MONITORPAGE_H
#define MONITORPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QByteArray>
#include <QTimer>
#include "basepage.h"
#include "mainwindow.h"
#include "led.h"
#include "speaker.h"
#include "gps.h"
#include "osrm.h"
#include "bluetooth.h"
#include "co2.h"

extern bool gestureLock;

namespace Ui {
  class MonitorPage;
}

class MainWindow;

class MonitorPage : public BasePage {
  Q_OBJECT

public:
  explicit MonitorPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket);
  ~MonitorPage();

  void activate() override;
  void deactivate() override;

private:
  Ui::MonitorPage* ui;
  MainWindow* mainWindow;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  int ciphertext_len = -1;
  quint8 cmd;
  qint64 lastAppendTime = 0;

  QTimer* wakeupTimer;
  bool wakeupFlashOn = false;
  bool wakeupFlashing = false;
  void wakeupUI(bool on);

  void readFrame();

  Led* led;
  Speaker* speaker;
  Bluetooth* bluetooth;
  Gps* gps;
  CO2Sensor* co2;
  int co2_v, temp, hum;
  QTimer* co2Timer;
  Osrm* osrm;
  double latitude, longitude;
  bool navigating = false;
  void navigation(bool on);

  void openDevice();
  void closeDevice();

  void playDevice();
  void stopDevice();
};

#endif // MONITORPAGE_H
