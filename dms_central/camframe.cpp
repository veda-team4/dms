#include "camframe.h"
#include "ui_camframe.h"
#include "connectdialog.h"
#include "rightframe.h"
#include "mainwindow.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

static unsigned char key[33] = "abcdefghijklmnopqrstuvwxyz012345";

int CamFrame::writeNBytes(QTcpSocket* socket, const void* buf, int len) {
    int totalWritten = 0;
    const char* buffer = static_cast<const char*>(buf);

    while (totalWritten < len) {
        qint64 bytesWritten = socket->write(buffer + totalWritten, len - totalWritten);

        if (bytesWritten < 0) {
            if (errno == EINTR)
                continue;
            perror("write");
            return -1;
        }

        if (bytesWritten == 0) {
            return -1; // 쓰기 불가
        }

        if (!socket->waitForBytesWritten(-1)) {  // 블로킹으로 보장
            return -1;
        }

        totalWritten += bytesWritten;
    }

    return totalWritten == len ? totalWritten : -1;
}

// writeEncryptedMessage: QTcpSocket으로 암호화 메시지 전송
int CamFrame::writeEncryptedMessage(std::string msg) {
    // 1. IV 생성
    unsigned char iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        qDebug() << "RAND_bytes failed\n";
        return -1;
    }

    // 2. 평문 준비
    const unsigned char *plaintext =
        reinterpret_cast<const unsigned char *>(msg.data());
    int plaintext_len = msg.size();

    // 3. 암호화
    unsigned char ciphertext[4096];
    int ciphertext_len;

    if (!aes_encrypt(plaintext, plaintext_len, key, iv, ciphertext,
                     &ciphertext_len)) {
        qDebug() << "AES encryption failed\n";
        return -1;
    }

    // 4. 암호문 길이
    uint32_t len_field = ciphertext_len;

    // 5. 순서대로 전송: [IV(16)] + [길이(4)] + [암호문]
    if (writeNBytes(socket, iv, 16) == -1)
        return -1;
    if (writeNBytes(socket, &len_field, 4) == -1)
        return -1;
    if (writeNBytes(socket, ciphertext, ciphertext_len) == -1)
        return -1;

    return 0;
}

CamFrame::CamFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::CamFrame)
{
    ui->setupUi(this);

    player = new QMediaPlayer(ui->videoLabel);
    videoWidget = new QVideoWidget(ui->videoLabel);
    socket = new QTcpSocket(this);

    videoWidget->setGeometry(ui->videoLabel->geometry().adjusted(1, 1, -1, -1));
    player->setVideoOutput(videoWidget);

    loadingGif = new QMovie(":/images/image/loading.gif");
    ui->loadingLabel->setMovie(loadingGif);

    connect(ui->connectButton, &QPushButton::clicked, this, &CamFrame::onButtonClicked);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CamFrame::onRtspChanged);
    connect(socket, &QTcpSocket::readyRead, this, &CamFrame::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &CamFrame::onDisconnected);
    connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {
        showFrame(1);
    });
    connect(ui->xbutton, &QPushButton::clicked, this, [&]() {
        resetPlayer();
        if (socket && socket->isOpen()) {
            socket->disconnectFromHost();
            socket->close();
        }
        name.clear();
        ip.clear();
        buffer.clear();
        ciphertext_len = -1;
        sleeping = false;
        over40 = false;
        showFrame(1);
    });

    showFrame(1);
    ui->topFrame->hide();
}

CamFrame::~CamFrame()
{
    delete loadingGif;
    delete ui;
    delete player;
    delete videoWidget;
    delete socket;
}

void CamFrame::setIpName(QString _name, QString _ip) {
    name = _name;
    ip = _ip;
}

void CamFrame::showFrame(int idx) {
    if (idx == 1) {
        ui->waitingFrame_1->show();
    }
    else {
        ui->waitingFrame_1->hide();
    }
    if (idx == 2) {
        ui->waitingFrame_2->show();
    }
    else {
        ui->waitingFrame_2->hide();
    }
    if (idx == 3) {
        ui->waitingFrame_3->show();
    }
    else {
        ui->waitingFrame_3->hide();
    }

    if (idx == 0) {
        ui->videoFrame->show();
        ui->topFrame->show();
        ui->nameipLabel->setText((name + " - " + ip));
    }
    else {
        ui->videoFrame->hide();
        ui->topFrame->hide();
    }
}

void CamFrame::onButtonClicked() {
    ConnectDialog dlg(mainWindow, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString ip = dlg.selectedIp();
        QString name = dlg.selectedName();
        setIpName(name, ip);

        socket->connectToHost(ip, 9000);

        showFrame(2);
        loadingGif->start();

        QString rtspUrl = QString("rtsps://admin:vedateam4@%1:8322/dms_stream").arg(ip);
        player->setSource(QUrl((rtspUrl)));
        player->play();
    }
}

void CamFrame::resetPlayer() {
    if (player) {
        player->stop();
        player->deleteLater();
    }
    player = new QMediaPlayer(this);
    videoWidget->setGeometry(ui->videoLabel->geometry());
    player->setVideoOutput(videoWidget);

    // 다시 시그널 연결
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &CamFrame::onRtspChanged);
}

void CamFrame::onRtspChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia) {
        loadingGif->stop();
        showFrame(0);
    }
    else if (status == QMediaPlayer::EndOfMedia) {
        resetPlayer();
        showFrame(1);
        name = ip = "";
    }
    else if (status == QMediaPlayer::InvalidMedia) {
        resetPlayer();
        player->setSource(QUrl());
        showFrame(3);
        name = ip = "";
    }
}

void CamFrame::onDisconnected() {
    buffer.clear();
    ciphertext_len = -1;
}

void CamFrame::sendMessage(QString str) {

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
                sleeping = true;
                ui->videoFrame->setStyleSheet("border: 1px solid red");
                mainWindow->rightFrame->addNewWidget(name, 3);
                return;
            }
            else if (cmd == Protocol::STRETCH) {
                sleeping = false;
                ui->videoFrame->setStyleSheet("border: none");
            }
            else {
                quint32 dataLen = *reinterpret_cast<const quint32*>(decrypted.constData() + 1);
                if (cmd == Protocol::EYECLOSEDRATIO) {
                    double value = *reinterpret_cast<const double*>(decrypted.constData() + 5);
                    int v = (int)(value * 100.0);
                    if (v < 40) {
                        if (!sleeping) {
                            ui->videoFrame->setStyleSheet("border: none;");
                        }
                        over40 = false;
                    }
                    else if (v < 80) {
                        if (!over40) {
                            over40 = true;
                            mainWindow->rightFrame->addNewWidget(name, 1);
                        }
                        if (!sleeping) {
                            ui->videoFrame->setStyleSheet("border: 1px solid yellow;");
                        }
                    }
                    else {
                        if (!sleeping) {
                            sleeping = true;
                            ui->videoFrame->setStyleSheet("border: 1px solid red;");
                            mainWindow->rightFrame->addNewWidget(name, 2);
                        }
                    }
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

bool CamFrame::aes_encrypt(const unsigned char* plaintext, int plaintext_len,
                 const unsigned char* key, const unsigned char* iv,
                 unsigned char* ciphertext, int* ciphertext_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    // 초기화 (AES-256-CBC, key/iv 설정)
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int len;

    // 평문 암호화
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    *ciphertext_len = len;

    // 패딩 처리 및 마무리
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    *ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return true;
}
