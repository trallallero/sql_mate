#include "filterfields.h"
#include "ui_filterfields.h"
#include "globals.h"

#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonDocument>

auto updateSelectAllTenantsCheckBox = [](QCheckBox* cb, QList<QCheckBox*>& tenantsCheckBoxes){
    auto totChecked = 0;
    for (auto& p : tenantsCheckBoxes)
    {
        if (p->isChecked())
            ++totChecked;
    }
    if (totChecked == 0)
        cb->setChecked(false);
    if (totChecked == tenantsCheckBoxes.size())
        cb->setChecked(true);
};

QStringList FilterFields::m_selectedFields {};
QStringList FilterFields::m_allFields      {};
//bool        FilterFields::m_firstTime      { true };

FilterFields::FilterFields(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::FilterFields)
{
    m_dialog = Globals::createShadowedGui(this);
    ui->setupUi(m_dialog);

    connectObjects();
    createFields();
}

FilterFields::~FilterFields()
{
    delete ui;
}

void FilterFields::createFields()
{
    setSelectedFields();
    setAllFields();

    for(auto& field : m_allFields)
    {
        auto cb = new QCheckBox(field, ui->groupBoxZeroFF);
        cb->setCursor(Qt::PointingHandCursor);
        ui->groupBoxZeroFF->layout()->addWidget(cb);
        if (m_selectedFields.contains(field))
            cb->setChecked(true);
        m_fieldsCheckBoxes.append(cb);
        connect(cb, &QCheckBox::toggled, this, [this](bool){
            updateSelectAllTenantsCheckBox(ui->checkBoxSelectTenants, m_fieldsCheckBoxes);
        });
    }
    updateSelectAllTenantsCheckBox(ui->checkBoxSelectTenants, m_fieldsCheckBoxes);
}

QStringList FilterFields::loadProfileSelectedFields()
{
    auto jsonObj = Globals::profileConfigurationObject();
    m_selectedFields = jsonObj["selectedFields"].toVariant().toStringList();
    return m_selectedFields;
}

void FilterFields::setSelectedFields()
{
    loadProfileSelectedFields();
}

QStringList FilterFields::getAllFields(bool lower)
{
    QStringList allFields;

    QFile inputFile(Globals::filenameProfileQuery());
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            auto field = getFieldFromLine(QString(in.readLine()));
            if(field.isEmpty() == false)
            {
                if (lower)
                    allFields.append(QString(field).toLower());
                else
                    allFields.append(QString(field));
            }
        }
    }
    std::sort(allFields.begin(), allFields.end());
    return allFields;
}

void FilterFields::setAllFields()
{
    m_allFields = getAllFields(false);
}

QString FilterFields::getFieldFromLine(QString line)
{
    QString field;
    std::sort(m_allFields.begin(), m_allFields.end());
    auto index = line.indexOf(" AS ");
    if (index >= 0)
    {
        auto firstIndex = index + 5;
        auto lastIndex  = line.indexOf("\"", firstIndex);
        field = line.mid(firstIndex, lastIndex - firstIndex);
    }
    return field;
}

void FilterFields::checkBoxSelectTenants_toggled(bool checked)
{
    for (auto& cb : m_fieldsCheckBoxes)
        cb->setChecked(checked);
}

void FilterFields::pushButtonOk_clicked()
{
    m_selectedFields.clear();

    for (auto& cb : m_fieldsCheckBoxes)
        if (cb->isChecked())
            m_selectedFields.append(cb->text());

    auto jsonObj = Globals::profileConfigurationObject();
    jsonObj["selectedFields"] = QJsonArray::fromStringList(m_selectedFields);

    QByteArray ba = QJsonDocument(jsonObj).toJson(QJsonDocument::JsonFormat::Indented);
    QFile fout(Globals::filenameProfileConfig());
    fout.open(QIODevice::WriteOnly);
    fout.write(ba);
    fout.close();

    emit fieldsSelected(m_selectedFields);

    accept();
}

void FilterFields::pushButtonCancel_clicked()
{
    reject();
}

void FilterFields::connectObjects()
{
    connect(ui->checkBoxSelectTenants, &QCheckBox::toggled  , this, &FilterFields::checkBoxSelectTenants_toggled);
    connect(ui->pushButtonOk         , &QPushButton::clicked, this, &FilterFields::pushButtonOk_clicked         );
    connect(ui->pushButtonCancel     , &QPushButton::clicked, this, &FilterFields::pushButtonCancel_clicked     );
}
