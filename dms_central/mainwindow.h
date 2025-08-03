#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "topframe.h"
#include "leftframe.h"
#include "centerframe.h"
#include "rightframe.h"
#include "camerainfo.h"
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    Ui::MainWindow *ui;
    TopFrame* topFrame;
    LeftFrame* leftFrame;
    CenterFrame* centerFrame;
    RightFrame* rightFrame;
    QString m_userID;
    QVector<CameraInfo> camerainfo;

public slots:
    void setId(const QString& id);

};
#endif // MAINWINDOW_H
