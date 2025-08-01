#ifndef CAMFRAME_H
#define CAMFRAME_H

#include <QFrame>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMovie>
#include <QTcpSocket>
#include <QLabel>

class MainWindow;

namespace Protocol {
enum Type: uint8_t {
    EYECLOSEDRATIO = 15,
    HEADDROPPED = 19,
    STRETCH = 22
};
}

namespace Ui {
class CamFrame;
}

class CamFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CamFrame(MainWindow* mainWindow, QWidget *parent = nullptr);
    ~CamFrame();
    void setIpName(QString _name, QString _ip);

private:
    Ui::CamFrame *ui;

public:
    MainWindow* mainWindow;
    QMediaPlayer* player = nullptr;
    QVideoWidget* videoWidget = nullptr;
    QMovie* loadingGif;
    QTcpSocket* socket;
    QByteArray buffer;
    QByteArray iv;
    QString name, ip;
    int ciphertext_len = -1;
    quint8 cmd;
    bool over40 = false;

    bool sleeping = false;

    void connectRtsp(QString ip);
    void resetPlayer();
    bool aes_decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, const unsigned char* iv, unsigned char* plaintext, int* plaintext_len);
    void showFrame(int idx);
    QLabel* overlayLabel;

private slots:
    void onButtonClicked();
    void onRtspChanged(QMediaPlayer::MediaStatus status);
    void onReadyRead();
    void onDisconnected();
};

#endif // CAMFRAME_H
