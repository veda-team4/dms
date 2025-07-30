#include "leftframe.h"
#include "ui_leftframe.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QWidget>
#include <adddriverdialog.h>
#include <ui_adddriverdialog.h>

LeftFrame::LeftFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::LeftFrame)
{
    ui->setupUi(this);

    // 1. 차량 노드 생성
    QTreeWidgetItem* carItem = new QTreeWidgetItem(ui->vehicleTree);
    //초기에도 왼쪽에 목록아이콘 생성
    carItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    // 2. 차량 이름 + '+' 버튼을 담을 위젯 구성
    QWidget* camWidget = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(camWidget);
    layout->setContentsMargins(4, 0, 4, 0);
    camWidget->setStyleSheet("background-color: transparent;");

    // // 2-1. 아이콘
    // QLabel* iconLabel = new QLabel;
    // QPixmap iconPixmap(":/images/image/Video_off.png");
    // iconLabel->setPixmap(iconPixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // iconLabel->setFixedSize(20, 20);  // 정렬 맞추기용

    // 2-2. 차량 라벨
    QLabel* camLabel = new QLabel("차량");
    camLabel->setStyleSheet("color: white; font: 10pt \"hanwhaGothic\";");

    // 2-3. +버튼
    QPushButton* addBtn = new QPushButton("+");
    addBtn->setFixedSize(20, 20);
    addBtn->setStyleSheet("color: white; font: 14pt \"hanwhaGothic\";");

    //layout->addWidget(iconLabel);
    layout->addWidget(camLabel);
    layout->addStretch();
    layout->addWidget(addBtn);

    camWidget->installEventFilter(this);

    ui->vehicleTree->setItemWidget(carItem, 0, camWidget);

    // 3. + 버튼 클릭 → 카메라 추가 다이얼로그 띄우기
    connect(addBtn, &QPushButton::clicked, this, [=]() {
        // 다이얼로그 팝업 띄우기
        AddDriverDialog dialog (mainWindow, this);
        if (dialog.exec() == QDialog::Accepted) {
            QString camName = dialog.dcamName();  // 입력된 카메라 이름

            // 트리 항목 추가
            QTreeWidgetItem* camItem = new QTreeWidgetItem(carItem);

            // 항목 내부 위젯 구성 (아이콘+ CAM 이름 + '–' 버튼)
            QWidget* camWidget = new QWidget;
            QHBoxLayout* camLayout = new QHBoxLayout(camWidget);
            camLayout->setContentsMargins(4, 0, 4, 0);
            camLayout->setSpacing(6);
            camWidget->setStyleSheet("background-color: transparent;");

            //아이콘
            QLabel* iconLabel = new QLabel;
            QPixmap iconPixmap(":/images/image/Video_on.png");
            iconLabel->setPixmap(iconPixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            iconLabel->setFixedSize(20, 20);  // 정렬 맞추기용
            iconLabel->setStyleSheet("background-color: transparent;");

            //라벨
            QLabel* camLabel = new QLabel(camName);
            camLabel->setStyleSheet("color: white; font: 10pt \"hanwhaGothic\";");

            //-버튼
            QPushButton* removeBtn = new QPushButton("–");
            // QPushButton* removeBtn = new QPushButton();
            // QPixmap iconPixmap2(":/images/image/remove_cam1.png");
            //removeBtn->setIcon(QIcon(iconPixmap));
            removeBtn->setFixedSize(20, 20);
            removeBtn->setStyleSheet("color: white; font: 12pt \"hanwhaGothic\";");

            camLayout->addWidget(iconLabel);
            camLayout->addWidget(camLabel);
            camLayout->addStretch();
            camLayout->addWidget(removeBtn);

            ui->vehicleTree->setItemWidget(camItem, 0, camWidget);

            // 삭제 버튼 연결
            connect(removeBtn, &QPushButton::clicked, this, [=]() {
                delete camItem;
            });

            carItem->setExpanded(true);
        }
    });

    //************스타일 시트 적용***************************//
    ui->vehicleTree->setStyleSheet(R"(
    /* 전체 배경 & 글자 */
    QTreeWidget {
        background-color: #1A1A1A;
        color: white;
        font-size: 10pt;
        font-weight: bold;
        border: transparent;
        outline: 0;
    }

    /* 트리 항목 줄 높이 */
    QTreeView::item {
        height: 32px;
        padding: 2px;
        border: none;
    }

    /* 선택된 항목 배경 */
    QTreeView::item:selected {
        background-color: #F37321;
        color: white;
    }

    /* 펼치기 아이콘 색상 조절 */
    QTreeView::branch:has-children:!has-siblings:closed,
    QTreeView::branch:closed:has-children {
        image: url(:/images/image/keyboard_arrow_right.png);
    }
    QTreeView::branch:open:has-children {
        image: url(:/images/image/keyboard_arrow_down.png);
    }

    /* 버튼 스타일 */
    QPushButton {
        background: transparent;
        border: transparent;
        color: white;
    }
    )");
}

LeftFrame::~LeftFrame()
{
    delete ui;
}

bool LeftFrame::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        // 모든 차량 노드 순회
        int carCount = ui->vehicleTree->topLevelItemCount();
        for (int i = 0; i < carCount; ++i) {
            QTreeWidgetItem* carItem = ui->vehicleTree->topLevelItem(i);
            for (int j = 0; j < carItem->childCount(); ++j) {
                QTreeWidgetItem* camItem = carItem->child(j);
                QWidget* camWidget = ui->vehicleTree->itemWidget(camItem, 0);
                if (camWidget == obj) {
                    ui->vehicleTree->setCurrentItem(camItem);  // 선택
                    return true;
                }
            }
        }
    }
    return QFrame::eventFilter(obj, event);
}

