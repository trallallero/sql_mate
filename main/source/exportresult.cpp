#include "exportresult.h"
#include "ui_exportresult.h"

#include "filterfields.h"
#include "globals.h"
#include "sql.h"

#include <QXlsx/header/xlsxdocument.h>
#include <QXlsx/header/xlsxchartsheet.h>
#include <QXlsx/header/xlsxcellrange.h>
#include <QXlsx/header/xlsxchart.h>
#include <QXlsx/header/xlsxrichstring.h>
#include <QXlsx/header/xlsxworkbook.h>

using namespace QXlsx;

#include <QJsonDocument>
#include <QFileDialog>

ExportResult::ExportResult(QWidget* parent, QTableWidget* tableWidget, ViewMode viewMode) :
    QDialog(parent)
  , ui(new Ui::ExportResult)
  , m_tableWidget(tableWidget)
  , m_viewMode(viewMode)
{
    auto dialog = Globals::createShadowedGui(this);
    ui->setupUi(dialog);

    connectObjects();
    setFromDefault();
}

ExportResult::~ExportResult()
{
    delete ui;
}

void ExportResult::pushButtonClose_clicked()
{
    close();
}

void ExportResult::pushButtonExport_clicked()
{
    ExportType type = ExportType::UNKNOWN;
    if (ui->checkBoxCSV->isChecked())
        type = ExportType::CSV;
    else if (ui->checkBoxExcel->isChecked())
        type = ExportType::EXCEL;
    else if (ui->checkBoxJSON->isChecked())
        type = ExportType::JSON;

    exportResult(type);
}

void ExportResult::exportResult(ExportType type)
{
    Q_ASSERT(m_tableWidget);

    SqlResultType valuesMapList;

    QMap<QString, QString> valuesMap;

    auto numFields = FilterFields::getSelectedFields().size();

    if (m_viewMode == VM_VERTICAL)
    {
        for(int row = 0, fieldNum = 1; row < m_tableWidget->rowCount(); ++row)
        {
            if (m_tableWidget->isRowHidden(row))
                continue;

            auto const& key   = m_tableWidget->item(row, 0)->text();
            auto const& value = m_tableWidget->item(row, 1)->text();

            valuesMap[key] = value;

            if (fieldNum++ == numFields)
            {
                valuesMapList.append(valuesMap);
                valuesMap.clear();
                fieldNum = 1;
                continue;
            }
        }
    }
    else
    {
        for(int row = 0; row < m_tableWidget->rowCount(); ++row)
        {
            if (m_tableWidget->isRowHidden(row))
                continue;

            for(int col = 0; col < m_tableWidget->columnCount(); ++col)
            {
                if (m_tableWidget->isColumnHidden(col))
                    continue;

                auto const& key   = m_tableWidget->horizontalHeaderItem(col)->text();
                auto const& value = m_tableWidget->item(row, col)->text();

                valuesMap[key] = value;
            }

            valuesMapList.append(valuesMap);
            valuesMap.clear();
        }
    }

    switch(type)
    {
        case ExportType::JSON:
            exportJSON(valuesMapList);
        break;
        case ExportType::CSV:
            exportCSV(valuesMapList);
        break;
        case ExportType::EXCEL:
            exportExcel(valuesMapList);
        break;
        default:
        break;
    }
}

void ExportResult::exportJSON(const QList<QMap<QString, QString>> valuesMapList)
{
    Q_ASSERT(valuesMapList.size() > 0);

    QString indentString = getIndentStringForJSON();

    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Esporta in formato JSON"),
                QDir::currentPath(),
                "JSON files (*.json) ;;");
    if(fileName.isNull())
        return;

    QFile f(fileName);
    f.open(QIODevice::WriteOnly);
    f.write("[");
    if (indentString.isEmpty() == false)
        f.write("\n");

    for(auto listItemIter = valuesMapList.begin(); listItemIter != valuesMapList.end();)
    {
        f.write("{");

        auto stdMap = listItemIter->toStdMap();
        for(auto iter = stdMap.begin(); iter != stdMap.end();)
        {
            f.write(QString("%1\"%2\" : \"%3\"").arg(indentString, iter->first, iter->second).toLocal8Bit());
            if (++iter != stdMap.end())
                f.write(",");
            if (indentString.isEmpty() == false)
                f.write("\n");
        }
        f.write("}");

        if (++listItemIter != valuesMapList.end())
            f.write(",");
        if (indentString.isEmpty() == false)
            f.write("\n");
    }
    f.write("]");
    f.close();
}

void ExportResult::exportCSV(const QList<QMap<QString, QString> > valuesMapList)
{
    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Esporta in formato CSV"),
                QDir::currentPath(),
                "CSV files (*.csv) ;;");
    if(fileName.isNull())
        return;

    QFile f(fileName);
    f.open(QIODevice::WriteOnly);

    auto delimiter = ui->comboBoxDelimiter->currentText();
    if (delimiter == "Tab")
        delimiter = "\t";

    auto stdMap = valuesMapList.first().toStdMap();
    for(auto iter = stdMap.begin(); iter != stdMap.end();)
    {
        f.write(QString("%1").arg(iter->first).toLocal8Bit());
        if (++iter != stdMap.end())
            f.write(QString("%1").arg(delimiter).toLocal8Bit());
    }
    f.write("\n");

    for(auto listItemIter = valuesMapList.begin(); listItemIter != valuesMapList.end(); ++listItemIter)
    {
        auto stdMap = listItemIter->toStdMap();
        for(auto iter = stdMap.begin(); iter != stdMap.end();)
        {
            f.write(QString("%1").arg(iter->second).toLocal8Bit());
            if (++iter != stdMap.end())
                f.write(QString("%1").arg(delimiter).toLocal8Bit());
        }
        f.write("\n");
    }
    f.close();
}

void ExportResult::exportExcel(const QList<QMap<QString, QString>> valuesMapList)
{
    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Esporta in formato Excel"),
                QDir::currentPath(),
                "Excel files (*.xlsx) ;;");
    if(fileName.isNull())
        return;

    QXlsx::Document xlsx;

    Format headerStyle;
    headerStyle.setFontSize(12);
    headerStyle.setFontBold(true);
    headerStyle.setFontColor(Qt::darkBlue);
    headerStyle.setHorizontalAlignment(Format::AlignHCenter);
    headerStyle.setVerticalAlignment(Format::AlignVCenter);

    int col = 1;
    auto stdMap = valuesMapList.first().toStdMap();
    for(auto iter = stdMap.begin(); iter != stdMap.end(); ++iter)
        xlsx.write(1, col++, iter->first, headerStyle);

    for(int y = 0; y < valuesMapList.size(); ++y)
    {
        auto stdMap = valuesMapList.at(y).toStdMap();
        int col = 1;
        for(auto iter = stdMap.begin(); iter != stdMap.end(); ++iter)
            xlsx.write(y + 2, col++, iter->second);
    }
    xlsx.autosizeColumnWidth();
    xlsx.saveAs(fileName);
}

void ExportResult::checkBoxJSON_toggled(bool checked)
{
    if(checked)
    {
        m_userChangedType = true;
        ui->groupBoxZeroIndent->setEnabled(checked);
        enableDefault("JSON");
    }
}

void ExportResult::checkBoxCSV_toggled(bool checked)
{
    if(checked)
    {
        m_userChangedType = true;
        ui->comboBoxDelimiter->setEnabled(checked);
        enableDefault("CSV");
    }
}

void ExportResult::checkBoxExcel_toggled(bool checked)
{
    if(checked)
    {
        m_userChangedType = true;
        enableDefault("Excel");
    }
}

void ExportResult::checkBoxTab_toggled(bool checked)
{
    ui->spinBoxSpaces->setEnabled(!checked);
}

void ExportResult::connectObjects()
{
    connect(ui->pushButtonClose , &QPushButton::clicked, this, &ExportResult::pushButtonClose_clicked );
    connect(ui->pushButtonExport, &QPushButton::clicked, this, &ExportResult::pushButtonExport_clicked);

    connect(ui->checkBoxJSON , &QCheckBox::toggled, this, &ExportResult::checkBoxJSON_toggled );
    connect(ui->checkBoxCSV  , &QCheckBox::toggled, this, &ExportResult::checkBoxCSV_toggled  );
    connect(ui->checkBoxExcel, &QCheckBox::toggled, this, &ExportResult::checkBoxExcel_toggled);
    connect(ui->checkBoxTab  , &QCheckBox::toggled, this, &ExportResult::checkBoxTab_toggled  );

    connect(ui->checkBoxSetDefault, &QCheckBox::toggled, this, [this](bool checked){
        if(checked)
        {
            if(m_userChangedType == false)
                setToDefault();
        }
    });
}

QString ExportResult::getIndentStringForJSON() const
{
    if (ui->checkBoxTab->isChecked())
        return "\t";
    if (ui->spinBoxSpaces->isEnabled() == false)
        return "";

    QString indentString;
    {
        for (auto i = 0; i < ui->spinBoxSpaces->text().toInt(); ++i)
            indentString.append(" ");
    }
    return indentString;
}

void ExportResult::setToDefault()
{
    auto configObj = Globals::globalConfigurationObject();

    QJsonObject jsonObj;
    if (ui->checkBoxCSV->isChecked())
    {
        jsonObj["type"]      = "CSV";
        jsonObj["delimiter"] = ui->comboBoxDelimiter->currentIndex();
    }
    else if (ui->checkBoxJSON->isChecked())
    {
        jsonObj["type"]   = "JSON";
        jsonObj["indent"] = ui->groupBoxZeroIndent->isChecked();
        if (ui->groupBoxZeroIndent->isChecked())
        {
            jsonObj["indent_type"] = ui->checkBoxTab->isChecked() ? "tab" : "space";
            if (ui->checkBoxTab->isChecked() == false)
                jsonObj["indent_spaces"] = ui->spinBoxSpaces->text();
        }
    }
    else
    {
        jsonObj["type"] = "Excel";
    }

    m_defaultType = jsonObj["type"].toString();

    configObj["defaultExport"] = jsonObj;

    Globals::saveGlobalConfigurationObject(configObj, this);
}

void ExportResult::setFromDefault()
{
    m_userChangedType = true; // to avoid useless savings

    auto configObj = Globals::globalConfigurationObject();
    auto jsonObj   = configObj["defaultExport"].toObject();
    m_defaultType  = jsonObj["type"].toString();
    if (m_defaultType == "CSV")
    {
        ui->checkBoxCSV->setChecked(true);
        ui->comboBoxDelimiter->setCurrentIndex(jsonObj["delimiter"].toInt(0));
        ui->checkBoxSetDefault->setChecked(true);
    }
    else if (m_defaultType == "JSON")
    {
        ui->checkBoxJSON->setChecked(true);
        ui->groupBoxZeroIndent->setChecked(jsonObj["indent"].toBool(false));
        if (ui->groupBoxZeroIndent->isChecked())
        {
            ui->checkBoxTab->setChecked(jsonObj["indent_type"].toString("") == "tab");
            if (ui->checkBoxTab->isChecked() == false)
            {
                ui->spinBoxSpaces->setValue(jsonObj["indent_spaces"].toInt(3));
            }
        }
    }
    else if (m_defaultType == "Excel" || m_defaultType.isEmpty())
    {
        ui->checkBoxExcel->setChecked(true);
    }

    enableDefault(m_defaultType);

    m_userChangedType = false;
}

void ExportResult::enableDefault(QString who)
{
    m_userChangedType = true;

    if (who == m_defaultType)
    {
        ui->checkBoxSetDefault->setChecked(true);
        ui->checkBoxSetDefault->setEnabled(false);
    }
    else
    {
        ui->checkBoxSetDefault->setChecked(false);
        ui->checkBoxSetDefault->setEnabled(true);
    }

    m_userChangedType = false;
}
