/********************************************************************************
** Form generated from reading UI file 'credenzialiyin.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CREDENZIALIYIN_H
#define UI_CREDENZIALIYIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CredenzialiYIN
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxCredenzialiYIN;
    QGridLayout *gridLayout;
    QLineEdit *lineEditSurname;
    QLabel *label_2;
    QLabel *label_7;
    QLabel *label;
    QComboBox *comboBoxType;
    QLineEdit *lineEditPwd;
    QLabel *label_10;
    QLineEdit *lineEditName;
    QDateEdit *dateEditRUI;
    QLabel *label_5;
    QLabel *label_9;
    QLineEdit *lineEditRUI;
    QLineEdit *lineEditEmail;
    QLabel *label_3;
    QLabel *label_6;
    QLineEdit *lineEditIntermediary;
    QLineEdit *lineEditManager;
    QDateEdit *dateEditBirth;
    QLabel *label_4;
    QLabel *label_8;
    QLabel *label_11;
    QComboBox *comboBoxSection;
    QGroupBox *groupBoxButtons;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonCreate;
    QPushButton *pushButtonCancel;

    void setupUi(QDialog *CredenzialiYIN)
    {
        if (CredenzialiYIN->objectName().isEmpty())
            CredenzialiYIN->setObjectName(QString::fromUtf8("CredenzialiYIN"));
        CredenzialiYIN->setWindowModality(Qt::ApplicationModal);
        CredenzialiYIN->resize(347, 437);
        verticalLayout = new QVBoxLayout(CredenzialiYIN);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxCredenzialiYIN = new QGroupBox(CredenzialiYIN);
        groupBoxCredenzialiYIN->setObjectName(QString::fromUtf8("groupBoxCredenzialiYIN"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBoxCredenzialiYIN->sizePolicy().hasHeightForWidth());
        groupBoxCredenzialiYIN->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(groupBoxCredenzialiYIN);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(-1, 30, -1, -1);
        lineEditSurname = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditSurname->setObjectName(QString::fromUtf8("lineEditSurname"));

        gridLayout->addWidget(lineEditSurname, 1, 2, 1, 1);

        label_2 = new QLabel(groupBoxCredenzialiYIN);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_7 = new QLabel(groupBoxCredenzialiYIN);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_7, 6, 0, 1, 1);

        label = new QLabel(groupBoxCredenzialiYIN);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        comboBoxType = new QComboBox(groupBoxCredenzialiYIN);
        comboBoxType->addItem(QString());
        comboBoxType->addItem(QString());
        comboBoxType->addItem(QString());
        comboBoxType->setObjectName(QString::fromUtf8("comboBoxType"));

        gridLayout->addWidget(comboBoxType, 7, 2, 1, 1);

        lineEditPwd = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditPwd->setObjectName(QString::fromUtf8("lineEditPwd"));

        gridLayout->addWidget(lineEditPwd, 6, 2, 1, 1);

        label_10 = new QLabel(groupBoxCredenzialiYIN);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy1.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_10, 9, 0, 1, 1);

        lineEditName = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditName->setObjectName(QString::fromUtf8("lineEditName"));

        gridLayout->addWidget(lineEditName, 0, 2, 1, 1);

        dateEditRUI = new QDateEdit(groupBoxCredenzialiYIN);
        dateEditRUI->setObjectName(QString::fromUtf8("dateEditRUI"));

        gridLayout->addWidget(dateEditRUI, 3, 2, 1, 1);

        label_5 = new QLabel(groupBoxCredenzialiYIN);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        label_9 = new QLabel(groupBoxCredenzialiYIN);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        sizePolicy1.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_9, 8, 0, 1, 1);

        lineEditRUI = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditRUI->setObjectName(QString::fromUtf8("lineEditRUI"));

        gridLayout->addWidget(lineEditRUI, 4, 2, 1, 1);

        lineEditEmail = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditEmail->setObjectName(QString::fromUtf8("lineEditEmail"));

        gridLayout->addWidget(lineEditEmail, 5, 2, 1, 1);

        label_3 = new QLabel(groupBoxCredenzialiYIN);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_6 = new QLabel(groupBoxCredenzialiYIN);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy1.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        lineEditIntermediary = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditIntermediary->setObjectName(QString::fromUtf8("lineEditIntermediary"));

        gridLayout->addWidget(lineEditIntermediary, 8, 2, 1, 1);

        lineEditManager = new QLineEdit(groupBoxCredenzialiYIN);
        lineEditManager->setObjectName(QString::fromUtf8("lineEditManager"));

        gridLayout->addWidget(lineEditManager, 9, 2, 1, 1);

        dateEditBirth = new QDateEdit(groupBoxCredenzialiYIN);
        dateEditBirth->setObjectName(QString::fromUtf8("dateEditBirth"));

        gridLayout->addWidget(dateEditBirth, 2, 2, 1, 1);

        label_4 = new QLabel(groupBoxCredenzialiYIN);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_8 = new QLabel(groupBoxCredenzialiYIN);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        sizePolicy1.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_8, 7, 0, 1, 1);

        label_11 = new QLabel(groupBoxCredenzialiYIN);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout->addWidget(label_11, 10, 0, 1, 1);

        comboBoxSection = new QComboBox(groupBoxCredenzialiYIN);
        comboBoxSection->addItem(QString());
        comboBoxSection->addItem(QString());
        comboBoxSection->addItem(QString());
        comboBoxSection->setObjectName(QString::fromUtf8("comboBoxSection"));

        gridLayout->addWidget(comboBoxSection, 10, 2, 1, 1);


        verticalLayout->addWidget(groupBoxCredenzialiYIN);

        groupBoxButtons = new QGroupBox(CredenzialiYIN);
        groupBoxButtons->setObjectName(QString::fromUtf8("groupBoxButtons"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(groupBoxButtons->sizePolicy().hasHeightForWidth());
        groupBoxButtons->setSizePolicy(sizePolicy2);
        horizontalLayout = new QHBoxLayout(groupBoxButtons);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButtonCreate = new QPushButton(groupBoxButtons);
        pushButtonCreate->setObjectName(QString::fromUtf8("pushButtonCreate"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushButtonCreate->sizePolicy().hasHeightForWidth());
        pushButtonCreate->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(pushButtonCreate);

        pushButtonCancel = new QPushButton(groupBoxButtons);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));
        sizePolicy3.setHeightForWidth(pushButtonCancel->sizePolicy().hasHeightForWidth());
        pushButtonCancel->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(pushButtonCancel);


        verticalLayout->addWidget(groupBoxButtons);

        QWidget::setTabOrder(lineEditName, lineEditSurname);
        QWidget::setTabOrder(lineEditSurname, dateEditBirth);
        QWidget::setTabOrder(dateEditBirth, dateEditRUI);
        QWidget::setTabOrder(dateEditRUI, lineEditRUI);
        QWidget::setTabOrder(lineEditRUI, lineEditEmail);
        QWidget::setTabOrder(lineEditEmail, lineEditPwd);
        QWidget::setTabOrder(lineEditPwd, comboBoxType);
        QWidget::setTabOrder(comboBoxType, lineEditIntermediary);
        QWidget::setTabOrder(lineEditIntermediary, lineEditManager);
        QWidget::setTabOrder(lineEditManager, pushButtonCreate);
        QWidget::setTabOrder(pushButtonCreate, pushButtonCancel);

        retranslateUi(CredenzialiYIN);

        QMetaObject::connectSlotsByName(CredenzialiYIN);
    } // setupUi

    void retranslateUi(QDialog *CredenzialiYIN)
    {
        CredenzialiYIN->setWindowTitle(QApplication::translate("CredenzialiYIN", "Credenziali YIN", nullptr));
        groupBoxCredenzialiYIN->setTitle(QApplication::translate("CredenzialiYIN", "Credenziali YIN", nullptr));
        label_2->setText(QApplication::translate("CredenzialiYIN", "Cognome", nullptr));
        label_7->setText(QApplication::translate("CredenzialiYIN", "Password", nullptr));
        label->setText(QApplication::translate("CredenzialiYIN", "Nome", nullptr));
        comboBoxType->setItemText(0, QApplication::translate("CredenzialiYIN", "None", nullptr));
        comboBoxType->setItemText(1, QApplication::translate("CredenzialiYIN", "Natural", nullptr));
        comboBoxType->setItemText(2, QApplication::translate("CredenzialiYIN", "Legal", nullptr));

        label_10->setText(QApplication::translate("CredenzialiYIN", "Manager", nullptr));
        label_5->setText(QApplication::translate("CredenzialiYIN", "RUI", nullptr));
        label_9->setText(QApplication::translate("CredenzialiYIN", "Intermediario", nullptr));
        label_3->setText(QApplication::translate("CredenzialiYIN", "Data nascita", nullptr));
        label_6->setText(QApplication::translate("CredenzialiYIN", "Email", nullptr));
        label_4->setText(QApplication::translate("CredenzialiYIN", "Data RUI", nullptr));
        label_8->setText(QApplication::translate("CredenzialiYIN", "Tipo", nullptr));
        label_11->setText(QApplication::translate("CredenzialiYIN", "Sezione", nullptr));
        comboBoxSection->setItemText(0, QApplication::translate("CredenzialiYIN", "A", nullptr));
        comboBoxSection->setItemText(1, QApplication::translate("CredenzialiYIN", "B", nullptr));
        comboBoxSection->setItemText(2, QApplication::translate("CredenzialiYIN", "E", nullptr));

        groupBoxButtons->setTitle(QString());
        pushButtonCreate->setText(QApplication::translate("CredenzialiYIN", "Create", nullptr));
        pushButtonCancel->setText(QApplication::translate("CredenzialiYIN", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CredenzialiYIN: public Ui_CredenzialiYIN {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CREDENZIALIYIN_H
