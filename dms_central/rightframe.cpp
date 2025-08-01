#include "rightframe.h"
#include "ui_rightframe.h"


RightFrame::RightFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::RightFrame)
{
    ui->setupUi(this);

    int yOffset = 0;

    connect(ui->alarm_button, &QPushButton::clicked, this, &RightFrame::alarmPage);
    connect(ui->event_button, &QPushButton::clicked, this, &RightFrame::eventPage);
    //connect(ui->search_button, &QPushButton::clicked, this, &RightFrame::searchEvent);
    connect(ui->send_button, &QPushButton::clicked, this, &RightFrame::inputMessage);
    //connect(ui->lineEdit, &QLineEdit::returnPressed, this, &RightFrame::inputMessage);
    connect(ui->textEdit, &QTextEdit::textChanged, this, [=]() {
        QString text = ui->textEdit->toPlainText();
        if (text.length() > 100) {
            text.truncate(100);
            ui->textEdit->setPlainText(text);
            QTextCursor cursor = ui->textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit->setTextCursor(cursor);
        }
    });

    connect(ui->event_button, &QPushButton::clicked, this, &RightFrame::addNewWidget);


    ui->scrollAreaWidgetContents->setMinimumHeight(yOffset);
    ui->scrollAreaWidgetContents->setMaximumHeight(yOffset);

    ui->combo_cam->addItem("  카메라 선택");
    ui->combo_cam->addItem("  CAM01");
    ui->combo_cam->addItem("  CAM02");
    ui->combo_cam->addItem("  CAM03");

    ui->complete->setVisible(false);

}

RightFrame::~RightFrame()
{
    delete ui;
    for(auto& widget: eventWidgets) {
        delete widget;
    }
}

/*
데이터 받기 - CAM 번호, 경고 종류, 경고 시간
*/

void RightFrame::eventPage() {
    ui->scrollArea->setVisible(true);
    ui->trans_alarm->setVisible(false);
    ui->trans_off->setVisible(true);
    ui->trans_on->setVisible(false);
    ui->event_on->setVisible(true);
    ui->event_off->setVisible(false);
}
void RightFrame::alarmPage() {
    ui->trans_alarm->setVisible(true);
    ui->scrollArea->setVisible(false);
    ui->trans_on->setVisible(true);
    ui->trans_off->setVisible(false);
    ui->event_off->setVisible(true);
    ui->event_on->setVisible(false);
}


void RightFrame::addNewWidget() {
    // 위젯 다 밀기
    for(auto& widget: eventWidgets) {
        widget->move(0, widget->y() + 100);
    }

    // 새로운 위젯 생성
    QWidget *newWidget = new QWidget(ui->scrollAreaWidgetContents);
    eventWidgets.push_back(newWidget);

    newWidget->setFixedSize(300, 100);
    newWidget->move(0, 0);

    newWidget->setStyleSheet(
        "background-color: transparent;"
        "border: none;"
        );

    // 위젯 안에 요소 추가
    QLabel *cam_label = new QLabel("CAM 00", newWidget);
    cam_label->setGeometry(25, 10, 100, 25);
    cam_label->setStyleSheet(
        "color: rgb(255, 255, 255);"
        "background-color: transparent;"
        "border: none;"  "font-size: 14px;"
        "font-family: HanwhaGothic;"
        );
    QLabel *icon = new QLabel("", newWidget);
    icon->setGeometry(30, 42, 16, 16);
    icon->setStyleSheet(
        "background-color: transparent;"
        "image: url(:/images/image/closing.png);"
        "border: none;"
        );
    static int i = 0;
    //QLabel *text_label = new QLabel("alarm text", newWidget);
    QLabel *text_label = new QLabel(QString::fromStdString(std::to_string(i++)), newWidget);
    text_label->setGeometry(52, 43, 130, 16);
    text_label->setStyleSheet(
        "color: rgb(176, 176, 176);"
        "background-color: transparent;"
        "border: none;" "font-size: 13px;"
        "font-family: HanwhaGothic;"
        );
    QLabel *time_label = new QLabel("2025.00.00 00:00:00", newWidget);
    time_label->setGeometry(45, 65, 160, 16);
    time_label->setStyleSheet(
        "color: rgb(176, 176, 176);"
        "background-color: transparent;"
        "border: none;" "font-size: 13px;"
        "font-family: HanwhaGothic;"
        );
    QFrame *line = new QFrame(newWidget);
    line->setGeometry(14, 90, 267, 1);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("background-color: rgb(44, 44, 44);");


    newWidget->show();

    // 다음 위젯 위치 계산
    yOffset += 100;

    // scrollAreaWidgetContents 높이 조정
    ui->scrollAreaWidgetContents->setMinimumHeight(yOffset);
    ui->scrollAreaWidgetContents->setMaximumHeight(yOffset);

}

void RightFrame::searchEvent() {

}

void RightFrame::inputMessage() {
    QString input = ui->textEdit->toPlainText();

    emit sendMessage(input);

    ui->textEdit->clear();
    ui->textEdit->setFocus();

    sendComplete();
}
/*
 * center
void RightFrame::receiveMessage(const QString &text) {
    qDebug() << "Message : " << text;
    //메시지 출력 함수
}

 * main
SenderWidget *sender = new SenderWidget;
Receiver *receiver = new Receiver;

QObject::connect(sender, &SenderWidget::textReadyToSend,
                 receiver, &Receiver::handleReceivedText);
*/

void RightFrame::sendComplete() {
    ui->complete->setVisible(true);
    //QTimer::singleShot(2000, this, SLOT(onDelayFinished()));
    //시간 주기
    ui->complete->setVisible(false);
}
void RightFrame::onDelayFinished() {}
