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
#include "basepage.h"

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

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void showStartPage();
  void showCamSetPage();
  void showCalibratePage();
  void showMonitorPage();

private:
  Ui::MainWindow* ui;

  QPoint m_dragStartPosition;
  bool m_isDragging;

  BasePage* startPage;
  BasePage* camSetPage;
  BasePage* calibratePage;
  BasePage* monitorPage;

  QProcess* serverProcess;
  QLocalSocket* socket;

  bool gestureLock = true;

  void focusMenu(int idx);
  void gestureImage(bool lock);
};
#endif // MAINWINDOW_H
