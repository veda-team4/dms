#include "reportpage.h"
#include "ui_reportpage.h"
#include "protocols.h"
#include "utils.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QGraphicsLayout>
#include <QVBoxLayout>
#include <QDateTime>

QT_CHARTS_USE_NAMESPACE

ReportPage::ReportPage(QWidget* parent, MainWindow* mainWindow, QLocalSocket* socket) :
    BasePage(parent), mainWindow(mainWindow), socket(socket),
    ui(new Ui::ReportPage)
{
    ui->setupUi(this);
    connect(ui->nextButton, &QPushButton::clicked, this, &ReportPage::moveToNext);
}

ReportPage::~ReportPage()
{
    delete timer;
    delete ui;
}

void ReportPage::activate() {
  connect(socket, &QLocalSocket::readyRead, this, &ReportPage::readFrame);
  writeEncryptedCommand(socket, Protocol::REPORT);
  printGraph(mainWindow->info);
  printSummary(mainWindow->info);
  playSleepingClip();
  if(!mainWindow->isLock()) {
      mainWindow->updateLock();
      writeEncryptedCommand(socket, Protocol::LOCK);
  }
  ui->sleepingTime_1->setVisible(false);
  ui->sleepingTime_2->setVisible(false);
}

void ReportPage::deactivate() {
  writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &ReportPage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
  if (timer) {
    timer->stop();
    delete timer;
    timer = nullptr;
  }
  mainWindow->sleepingFrames.clear();
}

void ReportPage::printGraph(Information& info) {
    QVector<double>& values = info.values;

    // === 1. 라인 시리즈 생성 ===
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < values.size(); ++i) {
        double x = (double)i / (values.size() - 1) * 100.0; // 0~100%
        double y = values[i];
        series->append(x, y);
    }

    // 라인 색상/굵기
    QPen pen(QColor("#F37321"));
    pen.setWidth(2);
    series->setPen(pen);

    // === 2. 차트 생성 ===
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->setBackgroundBrush(QColor("#0E1420"));  
    chart->setPlotAreaBackgroundVisible(false);
    chart->setMargins(QMargins(5, 5, 5, 5));
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundRoundness(0); // 여백 줄이기

    // === 3. X축 설정 ===
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 100);
    axisX->setLabelsVisible(false);
    axisX->setGridLineVisible(false);
    axisX->setLineVisible(true);
    axisX->setTickCount(2);

    QFont tinyFont;
    tinyFont.setPixelSize(1);
    axisX->setLabelsFont(tinyFont);

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // === 4. Y축 설정 ===
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 100);
    axisY->setTickCount(5);
    axisY->setLabelFormat("%d%%");
    axisY->setLabelsVisible(true);
    axisY->setLabelsBrush(QBrush(Qt::white));

    QFont labelFont;
    labelFont.setPixelSize(10);
    labelFont.setBold(true);
    axisY->setLabelsFont(labelFont);

    QPen gridPen(QColor(255, 255, 255, 80));
    axisY->setGridLinePen(gridPen);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // === 5. ChartView 생성 ===
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(0, 0, 0, 0);
    chartView->setStyleSheet("background: transparent;");

    // *** 꽉 채우기 위해 SizePolicy 강제 ***
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartView->setMinimumSize(0, 0);

    // === 6. 레이아웃에 추가 (여백 제거) ===
    QLayout *layout = ui->chartContainer->layout();
    if (!layout) {
        layout = new QVBoxLayout(ui->chartContainer);
        ui->chartContainer->setLayout(layout);
    }

    // 여백 및 간격 제거
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // 기존 차트 제거
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // 차트 추가
    layout->addWidget(chartView);
}

void ReportPage::printSummary(Information& info) {
    std::string averageStr = (std::to_string((int)info.sleepingAverage) + std::string("%"));
    ui->sleepingAverage->setText(QString::fromStdString(averageStr));
    std::string alertCount = (std::to_string(info.alertCount) + std::string("회"));
    ui->sleepingNum->setText(QString::fromStdString(alertCount));
    ui->drivingTime->setText(QString::fromStdString(info.drivingTime));
}

void ReportPage::playSleepingClip() {
    if (mainWindow->sleepingFrames.empty()) return;
    if (mainWindow->sleepingFrames.size() >= 1) {
        ui->novideo_1->setVisible(false);
        ui->sleepingTime_1->setVisible(true);
    }
    if (mainWindow->sleepingFrames.size() >= 2) {
        ui->novideo_2->setVisible(false);
        ui->sleepingTime_2->setVisible(true);
    }

    currentFrameIndex = 0;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
      if (mainWindow->sleepingFrames.empty()) return;
        auto& clip0 = mainWindow->sleepingFrames[mainWindow->sleepingFrames.size() - (mainWindow->sleepingFrames.size() >= 2 ? 2 : 1)];
        if (currentFrameIndex >= clip0.size()) {
            currentFrameIndex = 0;
        }
        // 프레임 출력
        ui->videoLabel->setPixmap(
            clip0[currentFrameIndex].scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)
        );
        // 졸음 시간 출력
        std::string t = "졸음 감지 #1 (" + mainWindow->sleepingTimes[mainWindow->sleepingTimes.size() - (mainWindow->sleepingTimes.size() >= 2 ? 2 : 1)] + ")";
        ui->sleepingTime_1->setText(QString::fromStdString(t));

        if (mainWindow->sleepingFrames.size() >= 2) {
            auto& clip1 = mainWindow->sleepingFrames[mainWindow->sleepingFrames.size() - 1];
            // 프레임 출력
            ui->videoLabel_2->setPixmap(
                clip1[currentFrameIndex].scaled(ui->videoLabel_2->size(), Qt::KeepAspectRatio)
            );
            // 졸음 시간 출력
            std::string t = "졸음 감지 #2 (" + mainWindow->sleepingTimes[mainWindow->sleepingTimes.size() - 1] + ")";
            ui->sleepingTime_2->setText(QString::fromStdString(t));
        }
        currentFrameIndex++;
    });
    timer->start(33); // 30fps 재생
}

void ReportPage::readFrame() {
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
      
      if (cmd == Protocol::RIGHT || cmd == Protocol::LEFT) {
        return;
      }
      else if (cmd == Protocol::STRETCH) {
          ui->nextButton->click();
        return;
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
