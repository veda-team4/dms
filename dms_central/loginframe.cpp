#include "loginframe.h"
#include "ui_loginframe.h"

LoginFrame::LoginFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::LoginFrame)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setPlaceholders();

    // 로그인 버튼 연동
    connect(ui->login_pushButton_2, &QPushButton::clicked,
            this, &LoginFrame::onLoginClicked);

    connect(ui->pw_lineEdit_2, &QLineEdit::returnPressed,
            this, &LoginFrame::onLoginClicked);

    // 창닫기 기능
    connect(ui->btn_close, &QPushButton::clicked, this, [this](){
        if(auto w = window())   w->close();
    });
}

void LoginFrame::setPlaceholders()
{
    ui->id_lineEdit_2->setPlaceholderText(QStringLiteral("아이디"));
    ui->pw_lineEdit_2-> setPlaceholderText(QStringLiteral("비밀번호"));

    ui->pw_lineEdit_2->setEchoMode(QLineEdit::Password);
}

void LoginFrame::onLoginClicked()
{
    m_id = ui->id_lineEdit_2->text();
    m_pw = ui->pw_lineEdit_2->text();

    bool ok = (m_id == "" && m_pw == "");
    if (ok) {
        emit loginSucceeded(m_id); // 로그인 성공 시그널
        this->close(); // 로그인 창 닫기
    } else {
        showLoginError();
    }
}

void LoginFrame::showLoginError()
{
    QMessageBox msg(nullptr);

    //msg.setWindowTitle(QStringLiteral("로그인 실패"));
    msg.setText(QStringLiteral("아이디/비밀번호를 확인하세요"));
    msg.setIcon(QMessageBox::Warning);

    msg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    // 다크 테마 + 버튼 색깔 커스텀
    msg.setStyleSheet(R"(
        QMessageBox {
            background-color: #1A1A1A;
            color: #EEE;
            border: 2px solid #2C2C2C;
        }
        QMessageBox QLabel {
            color: #FFFFFF;
            font-family: "hanwhaGothic";
            font-size: 11px;
            font-weight: 300;
        }
        QPushButton {
            background-color: #F37321;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #e3620f;
        }
    )");
    msg.exec();
}

/* 상단바 움직이는 이벤트 */
void LoginFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_drag    = true;
        m_dragPos = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
    }
}

void LoginFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drag)
        window()->move(event->globalPosition().toPoint() - m_dragPos);
}

void LoginFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_drag = false;
}
/*--------------------*/

LoginFrame::~LoginFrame()
{
    delete ui;
}
