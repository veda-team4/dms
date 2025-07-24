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
}

void ReportPage::deactivate() {
  writeEncryptedCommand(socket, Protocol::STOP);
  disconnect(socket, &QLocalSocket::readyRead, this, &ReportPage::readFrame);
  while (socket->waitForReadyRead(100) > 0) {
    socket->readAll();
  }
  buffer.clear();
  ciphertext_len = -1;
}

void ReportPage::printGraph(Information& info) {
    QVector<double>& values = info.values;

    QFont tinyFont;
    tinyFont.setPixelSize(1);

    // values.push_back(10); values.push_back(20); values.push_back(40); values.push_back(90); values.push_back(100); values.push_back(80);

    // === 2. 시리즈 생성 (인덱스를 0~100으로 정규화) ===
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < values.size(); ++i) {
        double x = (double)i / (values.size() - 1) * 100.0; // 0~100%
        double y = values[i];
        series->append(x, y);
    }

    QPen pen = series->pen();
    pen.setWidth(1);
    series->setPen(pen);

    // === 3. 차트 생성 ===
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->setMargins(QMargins(10, 0, 10, 0));  // 바깥쪽 여백 제거
    chart->layout()->setContentsMargins(0, 0, 0, 0); 

    // === 4. X축: 0~100% 고정 ===
    QValueAxis *axisX = new QValueAxis;
    axisX->setRange(0, 100);
    axisX->setLabelsVisible(false);
    axisX->setLabelsFont(tinyFont);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // === 5. Y축: 값 범위 ===
    QValueAxis *axisY = new QValueAxis;
    axisY->setRange(0, 100); // 값 범위 고정 (필요 시 min/max 계산)
    axisY->setLabelsVisible(false);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // === 6. QChartView를 chartContainer에 추가 ===
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setContentsMargins(0, 0, 0, 0);

    QLayout *layout = ui->chartContainer->layout();
    if (!layout) {
        layout = new QVBoxLayout(ui->chartContainer);
        ui->chartContainer->setLayout(layout);
    } else {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    }
    layout->addWidget(chartView);
}

void ReportPage::printSummary(Information& info) {
    std::string averageStr = "  운전 중 눈감김 비율 평균: ";
    averageStr += (std::to_string((int)info.sleepingAverage) + std::string("%"));
    ui->sleepingAverage->setText(QString::fromStdString(averageStr));
    std::string alertCount = "  졸음 경고 횟수: ";
    alertCount += (std::to_string(info.alertCount) + std::string("회"));
    ui->alertCount->setText(QString::fromStdString(alertCount));
}

void ReportPage::playSleepingClip() {
    if (mainWindow->sleepingFrames.empty()) return;

    currentClipIndex = currentFrameIndex = 0;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        auto &clip = mainWindow->sleepingFrames[currentClipIndex];
        ui->sleepingLabel->setText(QString::fromStdString(std::string("졸음 영상 #") + std::to_string(currentClipIndex + 1)));
        if (currentFrameIndex >= clip.size()) {
            // 다음 클립으로
            currentClipIndex++;
            currentFrameIndex = 0;

            if (currentClipIndex >= mainWindow->sleepingFrames.size()) {
                currentClipIndex = 0;
                currentFrameIndex = 0;
                return;
            }
        }

        // 프레임 출력
        ui->videoLabel->setPixmap(
            clip[currentFrameIndex].scaled(ui->videoLabel->size(), Qt::KeepAspectRatio)
        );

        currentFrameIndex++;
    });

    timer->start(33); // 30fps 재생
}

void ReportPage::readFrame() {

}
