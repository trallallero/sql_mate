/********************************************************************************
** Form generated from reading UI file 'testframe.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TESTFRAME_H
#define UI_TESTFRAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TestFrame
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxZero;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QGroupBox *groupBoxZero_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelCredits;
    QGroupBox *groupBoxZero_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButtonClose;

    void setupUi(QFrame *TestFrame)
    {
        if (TestFrame->objectName().isEmpty())
            TestFrame->setObjectName(QString::fromUtf8("TestFrame"));
        TestFrame->resize(342, 565);
        TestFrame->setAutoFillBackground(false);
        verticalLayout = new QVBoxLayout(TestFrame);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxZero = new QGroupBox(TestFrame);
        groupBoxZero->setObjectName(QString::fromUtf8("groupBoxZero"));
        horizontalLayout = new QHBoxLayout(groupBoxZero);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBoxZero);
        label->setObjectName(QString::fromUtf8("label"));
        label->setText(QString::fromUtf8(""));
        label->setPixmap(QPixmap(QString::fromUtf8(":/images/sqlmate.ico")));
        label->setScaledContents(true);

        horizontalLayout->addWidget(label);


        verticalLayout->addWidget(groupBoxZero);

        groupBoxZero_2 = new QGroupBox(TestFrame);
        groupBoxZero_2->setObjectName(QString::fromUtf8("groupBoxZero_2"));
        horizontalLayout_3 = new QHBoxLayout(groupBoxZero_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        labelCredits = new QLabel(groupBoxZero_2);
        labelCredits->setObjectName(QString::fromUtf8("labelCredits"));

        horizontalLayout_3->addWidget(labelCredits);


        verticalLayout->addWidget(groupBoxZero_2);

        groupBoxZero_3 = new QGroupBox(TestFrame);
        groupBoxZero_3->setObjectName(QString::fromUtf8("groupBoxZero_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBoxZero_3->sizePolicy().hasHeightForWidth());
        groupBoxZero_3->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(groupBoxZero_3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        pushButtonClose = new QPushButton(groupBoxZero_3);
        pushButtonClose->setObjectName(QString::fromUtf8("pushButtonClose"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButtonClose->sizePolicy().hasHeightForWidth());
        pushButtonClose->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(pushButtonClose);


        verticalLayout->addWidget(groupBoxZero_3);


        retranslateUi(TestFrame);

        QMetaObject::connectSlotsByName(TestFrame);
    } // setupUi

    void retranslateUi(QFrame *TestFrame)
    {
        TestFrame->setWindowTitle(QApplication::translate("TestFrame", "Frame", nullptr));
        groupBoxZero->setTitle(QString());
        groupBoxZero_2->setTitle(QString());
        labelCredits->setText(QString());
        groupBoxZero_3->setTitle(QString());
        pushButtonClose->setText(QApplication::translate("TestFrame", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TestFrame: public Ui_TestFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TESTFRAME_H
