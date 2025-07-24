#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QProcess>
#include <QLocalSocket>
#include <QMouseEvent>
#include <QPoint>
#include "startpage.h"
#include "camsetpage.h"
#include "calibratepage.h"
#include "monitorpage.h"
#include "reportpage.h"
#include "basepage.h"

struct Information {
  QVector<double> values;
  int alertCount = 0;
  unsigned long long sleepingCount = 0;
  double sleepingAverage = 0.0;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  void updateLock();
  bool isLock();
  Information info;

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void showStartPage();
  void showCamSetPage();
  void showCalibratePage();
  void showMonitorPage();
  void showReportPage();

private:
  Ui::MainWindow* ui;

  QPoint m_dragStartPosition;
  bool m_isDragging;

  BasePage* startPage;
  BasePage* camSetPage;
  BasePage* calibratePage;
  BasePage* monitorPage;
  BasePage* reportPage;

  QProcess* serverProcess;
  QLocalSocket* socket;

  bool gestureLock = true;


  void focusMenu(int idx);
  void gestureImage(bool lock);
};
#endif // MAINWINDOW_H
