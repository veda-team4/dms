#ifndef CALIBRATEPAGE_H
#define CALIBRATEPAGE_H

#include <QWidget>
#include <QLocalSocket>
#include <QBuffer>
#include <QPixmap>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include "basepage.h"
#include "mainwindow.h"
#include "speaker.h"

namespace Ui {
  class CalibratePage;
}

class MainWindow;

class CalibratePage : public BasePage {
  Q_OBJECT

public:
  explicit CalibratePage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket);
  ~CalibratePage();

  void activate() override;
  void deactivate() override;

private slots:
  void moveToNextStep();
  void moveToPreviousStep();

private:
  Ui::CalibratePage* ui;
  MainWindow* mainWindow;
  QLocalSocket* socket;
  QByteArray buffer;
  QByteArray iv;
  QTimer* finishTimer;
  int ciphertext_len = -1;
  quint8 cmd;
  void readFrame();
  int clickCount = 0;
  int progressStep = 0;
  const int progressTotalSteps = 20;
  Speaker* speaker;
};

#endif // CALIBRATEPAGE_H
