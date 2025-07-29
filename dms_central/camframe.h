#ifndef CAMFRAME_H
#define CAMFRAME_H

#include <QFrame>

namespace Ui {
class CamFrame;
}

class CamFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CamFrame(QWidget *parent = nullptr);
    ~CamFrame();

private:
    Ui::CamFrame *ui;
};

#endif // CAMFRAME_H
