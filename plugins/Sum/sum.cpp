#include "sum.h"
#include "ui_sum.h"

#include "globals.h"

#include <regex>

#include <QClipboard>

Sum::Sum()
    : ui(new Ui::Sum)
{
}

Sum::~Sum()
{
    if (ui)
        delete ui;
}

QString Sum::getName() const
{
    return QString("Sum");
}

void Sum::execute()
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

    ui->labelSum->clear();

    double sum      = 0.0;
    auto ok         = true;
    auto isDouble   = false;
    auto values     = getData()["value"].toString();
    auto valuesList = values.split(Globals::valuesSeparator(), QString::SkipEmptyParts);
    for(auto& value : valuesList)
    {
        if(isDouble == false && value.contains("."))
            isDouble = true;
        sum += value.toDouble(&ok);
        if(ok == false)
            break;
    }
    if(ok)
    {
        if(isDouble)
            ui->labelSum->setText(QString::number(sum, 'f', 2));
        else
            ui->labelSum->setText(QString::number(sum, 'f', 0));
    }
    else
    {
        ui->labelSum->setText(tr("C'è stato un errore!"));
    }
    exec();
}

bool Sum::isEnabled()
{
    auto values     = getData()["value"].toString();
    auto valuesList = values.split(Globals::valuesSeparator(), QString::SkipEmptyParts);
    if(valuesList.count() < 2)
        return false;

    for(auto& value : valuesList)
        if(isNumber(value.toStdString()) == false)
            return false;

    return true;
}

bool Sum::isNumber(std::string str)
{
    return std::regex_match(str, std::regex( ( "((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?")));
}

void Sum::pushButtonClose_clicked()
{
    ui->labelSum->clear();
    close();
}

void Sum::toolButtonCopy_clicked()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->labelSum->text());
    m_messageMethod(tr("Valore copiato"));
}

void Sum::connectObjects()
{
    connect(ui->pushButtonClose, &QPushButton::clicked, this, &Sum::pushButtonClose_clicked);
    connect(ui->toolButtonCopy , &QToolButton::clicked, this, &Sum::toolButtonCopy_clicked );
}

