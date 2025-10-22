#include "createcondition.h"
#include "ui_createcondition.h"

#include "globals.h"

#include <QApplication>
#include <QWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>

CreateCondition::CreateCondition()
    : ui(new Ui::CreateCondition)
{
}

CreateCondition::~CreateCondition()
{
    if (ui)
        delete ui;
}

QString CreateCondition::getName() const
{
    return QString("Create condition");
}

void CreateCondition::execute()
{
    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        m_uiSetup = true;

        connectObjects();

        auto stylesheet = getData()["stylesheet"].toString();
        QWidget::setStyleSheet(stylesheet);
    }

    if(languageChanged())
    {
        ui->retranslateUi(this);
        setLanguageChanged(false);
    }

    auto title   = getData()["title"  ].toString();
    auto value   = getData()["value"  ].toString();

    auto fieldsFreeLeft  = getData()["fieldsFreeLeft" ].toInt();
    auto fieldsFreeRight = getData()["fieldsFreeRight"].toInt();

    uncheckRadioButtons();

    ui->radioButtonLeft ->setEnabled(fieldsFreeLeft  > 0);
    ui->radioButtonRight->setEnabled(fieldsFreeRight > 0);

    ui->pushButtonCreate->setEnabled(false);

    ui->labelSelectedField->setText(title);
    ui->lineEditValue     ->setText(value);

    ui->lineEditValue->setFocus();

    exec();
}

bool CreateCondition::isEnabled()
{
    auto title = getData()["title"].toString().toLower();
    auto conditionNames = getData()["conditionNames"].toArray().toVariantList();

    return title.isEmpty() == false && title != "tenant" && conditionNames.contains(title) == false;
}

void CreateCondition::pushButtonCancel_clicked()
{
    close();
}

void CreateCondition::setConditionFromTitle(QString filename, QString title)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            auto line = QString(in.readLine());
            auto upperLine = line.toUpper();
            if (upperLine.contains(QString("AS \"%1\"").arg(title.toUpper())))
            {
                auto quote = ui->radioButtonString->isChecked() ? "'" : "";
                auto str = line.split(" AS ", QString::SkipEmptyParts).at(0).trimmed();
                m_conditions = QString("AND %1 = %2{%3}%4").arg(str, quote, title, quote);
            }
        }
        inputFile.close();
    }
}

void CreateCondition::uncheckRadioButtons()
{
    ui->radioButtonLeft->setAutoExclusive(false);
    ui->radioButtonLeft->setChecked(false);
    ui->radioButtonLeft->setAutoExclusive(true);

    ui->radioButtonRight->setAutoExclusive(false);
    ui->radioButtonRight->setChecked(false);
    ui->radioButtonRight->setAutoExclusive(true);
}

void CreateCondition::pushButtonCreate_clicked()
{
    QJsonObject jo;

    auto profile = getData()["profile"].toString();
    auto value   = ui->lineEditValue->text();
    auto title   = getData()["title"].toString();

    setConditionFromTitle(Globals::filenameProfileQuery(profile), title);

    jo["title"     ] = title;
    jo["value"     ] = value;
    jo["page"      ] = ui->radioButtonLeft->isChecked() ? 0 : 1,
    jo["conditions"] = QJsonArray{m_conditions};
    jo["not"       ] = ui->checkBoxNOT ->isChecked();
    jo["null"      ] = ui->checkBoxNULL->isChecked();
    jo["in"        ] = ui->checkBoxIN  ->isChecked();
    jo["like"      ] = ui->checkBoxLIKE->isChecked();

    QTimer::singleShot(0, this, [this, jo](){
        m_addConditionMethod(jo);
        if(ui->checkBoxExecute->isChecked())
            m_searchMethod();
    });

    close();
}

void CreateCondition::checkBoxIN_toggled(bool checked)
{
    ui->checkBoxLIKE->setEnabled(!checked);
    ui->checkBoxNULL->setEnabled(!checked);
}

void CreateCondition::checkBoxLIKE_toggled(bool checked)
{
    ui->checkBoxIN  ->setEnabled(!checked);
    ui->checkBoxNULL->setEnabled(!checked);
}

void CreateCondition::checkBoxNULL_toggled(bool checked)
{
    ui->checkBoxLIKE->setEnabled(!checked);
    ui->checkBoxIN  ->setEnabled(!checked);
}

void CreateCondition::connectObjects()
{
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &CreateCondition::pushButtonCancel_clicked);
    connect(ui->pushButtonCreate, &QPushButton::clicked, this, &CreateCondition::pushButtonCreate_clicked);

    connect(ui->checkBoxIN  , &QCheckBox::toggled, this, &CreateCondition::checkBoxIN_toggled  );
    connect(ui->checkBoxLIKE, &QCheckBox::toggled, this, &CreateCondition::checkBoxLIKE_toggled);
    connect(ui->checkBoxNULL, &QCheckBox::toggled, this, &CreateCondition::checkBoxNULL_toggled);

    connect(ui->radioButtonLeft , &QRadioButton::clicked, this, [this](){ ui->pushButtonCreate->setEnabled(true); });
    connect(ui->radioButtonRight, &QRadioButton::clicked, this, [this](){ ui->pushButtonCreate->setEnabled(true); });
}
