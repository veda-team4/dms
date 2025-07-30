#include "camframe.h"
#include "ui_camframe.h"
#include "connectdialog.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

static unsigned char key[33] = "abcdefghijklmnopqrstuvwxyz012345";

CamFrame::CamFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::CamFrame)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(this);
    socket = new QTcpSocket(this);

    videoWidget->setGeometry(ui->videoLabel->geometry());
    player->setVideoOutput(videoWidget);

    loadingGif = new QMovie(":/images/image/loading.gif");
    ui->loadingLabel->setMovie(loadingGif);
    ui->waitingFrame_1->show();
    ui->waitingFrame_2->hide();
    videoWidget->hide();

    connect(ui->connectButton, &QPushButton::clicked, this, &CamFrame::onButtonClicked);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CamFrame::onRtspChanged);
    connect(socket, &QTcpSocket::readyRead, this, &CamFrame::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &CamFrame::onDisconnected);
}

CamFrame::~CamFrame()
{
    delete loadingGif;
    delete ui;
    delete player;
    delete videoWidget;
    delete socket;
}

void CamFrame::onButtonClicked() {
    ConnectDialog dlg(mainWindow, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString ip = dlg.selectedIp();

        socket->connectToHost(ip, 9000);

        ui->waitingFrame_1->hide();
        ui->waitingFrame_2->show();
        ui->waitingFrame_2->raise();
        loadingGif->start();

        QString rtspUrl = QString("rtsp://%1:8554/mystream").arg(ip);
        player->setSource(QUrl((rtspUrl)));
        player->play();
    }
}

void CamFrame::onRtspChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia) {
        loadingGif->stop();
        ui->waitingFrame_2->hide();
        videoWidget->show();
        videoWidget->raise();
    }
    else if (status == QMediaPlayer::EndOfMedia) {
        videoWidget->hide();
        ui->waitingFrame_1->show();
        ui->waitingFrame_1->raise();
    }
}

void CamFrame::onDisconnected() {
    buffer.clear();
    ciphertext_len = -1;
}

void CamFrame::onReadyRead() {
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

            aes_decrypt(
                reinterpret_cast<const unsigned char*>(encrypted.constData()), encrypted.size(),
                key, reinterpret_cast<const unsigned char*>(iv.constData()),
                reinterpret_cast<unsigned char*>(decrypted.data()), &decrypted_len
                );

            // 복호화된 평문에서 명령과 길이 추출
            quint8 cmd = static_cast<quint8>(decrypted[0]);

            if (cmd == Protocol::HEADDROPPED) {
                return;
            }
            else {
                quint32 dataLen = *reinterpret_cast<const quint32*>(decrypted.constData() + 1);
                if (cmd == Protocol::EYECLOSEDRATIO) {
                    double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
                    int v = (int)(value * 100.0);
                    return;
                }
            }
        }
        else {
            break;  // 아직 데이터 부족
        }
    }
}

bool CamFrame::aes_decrypt(const unsigned char* ciphertext, int ciphertext_len,
                 const unsigned char* key, const unsigned char* iv,
                 unsigned char* plaintext, int* plaintext_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int len;
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    *plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    *plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return true;
}
