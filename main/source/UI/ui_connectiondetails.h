/********************************************************************************
** Form generated from reading UI file 'connectiondetails.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTIONDETAILS_H
#define UI_CONNECTIONDETAILS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ConnectionDetails
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxConnections;
    QGridLayout *gridLayout;
    QLabel *labelUser;
    QLineEdit *lineEditDatabase;
    QLabel *labelPassword;
    QLineEdit *lineEditHost;
    QLabel *labelDatabase;
    QLineEdit *lineEditUser;
    QLabel *labelHost;
    QLineEdit *lineEditPassword;
    QLabel *labelName;
    QLineEdit *lineEditName;
    QGroupBox *groupBoxButtons;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonSave;
    QPushButton *pushButtonCancel;

    void setupUi(QDialog *ConnectionDetails)
    {
        if (ConnectionDetails->objectName().isEmpty())
            ConnectionDetails->setObjectName(QString::fromUtf8("ConnectionDetails"));
        ConnectionDetails->setWindowModality(Qt::ApplicationModal);
        ConnectionDetails->resize(400, 373);
        verticalLayout = new QVBoxLayout(ConnectionDetails);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBoxConnections = new QGroupBox(ConnectionDetails);
        groupBoxConnections->setObjectName(QString::fromUtf8("groupBoxConnections"));
        gridLayout = new QGridLayout(groupBoxConnections);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        labelUser = new QLabel(groupBoxConnections);
        labelUser->setObjectName(QString::fromUtf8("labelUser"));

        gridLayout->addWidget(labelUser, 3, 0, 1, 1);

        lineEditDatabase = new QLineEdit(groupBoxConnections);
        lineEditDatabase->setObjectName(QString::fromUtf8("lineEditDatabase"));

        gridLayout->addWidget(lineEditDatabase, 2, 1, 1, 1);

        labelPassword = new QLabel(groupBoxConnections);
        labelPassword->setObjectName(QString::fromUtf8("labelPassword"));

        gridLayout->addWidget(labelPassword, 4, 0, 1, 1);

        lineEditHost = new QLineEdit(groupBoxConnections);
        lineEditHost->setObjectName(QString::fromUtf8("lineEditHost"));

        gridLayout->addWidget(lineEditHost, 1, 1, 1, 1);

        labelDatabase = new QLabel(groupBoxConnections);
        labelDatabase->setObjectName(QString::fromUtf8("labelDatabase"));

        gridLayout->addWidget(labelDatabase, 2, 0, 1, 1);

        lineEditUser = new QLineEdit(groupBoxConnections);
        lineEditUser->setObjectName(QString::fromUtf8("lineEditUser"));

        gridLayout->addWidget(lineEditUser, 3, 1, 1, 1);

        labelHost = new QLabel(groupBoxConnections);
        labelHost->setObjectName(QString::fromUtf8("labelHost"));

        gridLayout->addWidget(labelHost, 1, 0, 1, 1);

        lineEditPassword = new QLineEdit(groupBoxConnections);
        lineEditPassword->setObjectName(QString::fromUtf8("lineEditPassword"));
        lineEditPassword->setEchoMode(QLineEdit::Password);
        lineEditPassword->setClearButtonEnabled(false);

        gridLayout->addWidget(lineEditPassword, 4, 1, 1, 1);

        labelName = new QLabel(groupBoxConnections);
        labelName->setObjectName(QString::fromUtf8("labelName"));

        gridLayout->addWidget(labelName, 0, 0, 1, 1);

        lineEditName = new QLineEdit(groupBoxConnections);
        lineEditName->setObjectName(QString::fromUtf8("lineEditName"));

        gridLayout->addWidget(lineEditName, 0, 1, 1, 1);


        verticalLayout->addWidget(groupBoxConnections);

        groupBoxButtons = new QGroupBox(ConnectionDetails);
        groupBoxButtons->setObjectName(QString::fromUtf8("groupBoxButtons"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBoxButtons->sizePolicy().hasHeightForWidth());
        groupBoxButtons->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(groupBoxButtons);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButtonSave = new QPushButton(groupBoxButtons);
        pushButtonSave->setObjectName(QString::fromUtf8("pushButtonSave"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButtonSave->sizePolicy().hasHeightForWidth());
        pushButtonSave->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(pushButtonSave);

        pushButtonCancel = new QPushButton(groupBoxButtons);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));
        sizePolicy1.setHeightForWidth(pushButtonCancel->sizePolicy().hasHeightForWidth());
        pushButtonCancel->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(pushButtonCancel);


        verticalLayout->addWidget(groupBoxButtons);

        QWidget::setTabOrder(lineEditName, lineEditHost);
        QWidget::setTabOrder(lineEditHost, lineEditDatabase);
        QWidget::setTabOrder(lineEditDatabase, lineEditUser);
        QWidget::setTabOrder(lineEditUser, lineEditPassword);
        QWidget::setTabOrder(lineEditPassword, pushButtonSave);
        QWidget::setTabOrder(pushButtonSave, pushButtonCancel);

        retranslateUi(ConnectionDetails);

        QMetaObject::connectSlotsByName(ConnectionDetails);
    } // setupUi

    void retranslateUi(QDialog *ConnectionDetails)
    {
        ConnectionDetails->setWindowTitle(QApplication::translate("ConnectionDetails", "Connection Details", nullptr));
        groupBoxConnections->setTitle(QApplication::translate("ConnectionDetails", "Connection Details", nullptr));
        labelUser->setText(QApplication::translate("ConnectionDetails", "User", nullptr));
        labelPassword->setText(QApplication::translate("ConnectionDetails", "Password", nullptr));
        labelDatabase->setText(QApplication::translate("ConnectionDetails", "Database", nullptr));
        labelHost->setText(QApplication::translate("ConnectionDetails", "Host", nullptr));
        labelName->setText(QApplication::translate("ConnectionDetails", "Name", nullptr));
        groupBoxButtons->setTitle(QString());
        pushButtonSave->setText(QApplication::translate("ConnectionDetails", "Save", nullptr));
        pushButtonCancel->setText(QApplication::translate("ConnectionDetails", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ConnectionDetails: public Ui_ConnectionDetails {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTIONDETAILS_H
