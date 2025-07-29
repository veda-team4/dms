#include "leftframe.h"
#include "ui_leftframe.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidgetItem>
#include <QWidget>
#include <connectdialog.h>
#include <ui_connectdialog.h>

LeftFrame::LeftFrame(MainWindow* mainWindow, QWidget *parent)
    : mainWindow(mainWindow), QFrame(parent)
    , ui(new Ui::LeftFrame)
{
    ui->setupUi(this);

    // 1. 차량 노드 생성
    QTreeWidgetItem* carItem = new QTreeWidgetItem(ui->vehicleTree);
    carItem->setText(0, "차량 01");
    //초기에도 왼쪽에 목록아이콘 생성
    carItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    // 2. 차량 이름 + + 버튼을 담을 위젯 구성
    QWidget* carWidget = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(carWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* carLabel = new QLabel("차량 01");
    QPushButton* addBtn = new QPushButton("+");
    addBtn->setFixedSize(20, 20);

    layout->addWidget(carLabel);
    layout->addStretch();
    layout->addWidget(addBtn);

    ui->vehicleTree->setItemWidget(carItem, 0, carWidget);

    // 3. + 버튼 클릭 → 카메라 추가 다이얼로그 띄우기
    connect(addBtn, &QPushButton::clicked, this, [=]() {
        // 다이얼로그 팝업 띄우기
        /*
        ConnectDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString camName = dialog.getName();  // 입력된 카메라 이름

            // 트리 항목 추가
            QTreeWidgetItem* camItem = new QTreeWidgetItem(carItem);
            camItem->setText(0, camName);

            // 항목 내부 위젯 구성 (CAM 이름 + – 버튼)
            QWidget* camWidget = new QWidget;
            QHBoxLayout* camLayout = new QHBoxLayout(camWidget);
            camLayout->setContentsMargins(0, 0, 0, 0);

            QLabel* camLabel = new QLabel(camName);
            QPushButton* removeBtn = new QPushButton("–");
            removeBtn->setFixedSize(20, 20);

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
        */
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

    QWidget {
        color: white;
        border: transparent;
        background-color: #1A1A1A;
        font: 10pt "hanwhaGothic";
    }

    /* 트리 항목 줄 높이 */
    QTreeView::item {
        height: 32px;
        padding: 2px;
    }

    /* 선택된 항목 배경 */
    QTreeView::item:selected {
        background-color: #1A1A1A;       /* #f37321; */
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

    QPushButton:hover {
        background-color: #333333;
    }
    )");

}

LeftFrame::~LeftFrame()
{
    delete ui;
}
