#include "selectfromtable.h"
#include "ui_selectfromtable.h"

#include "widgetsizetool.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QTimer>

#include "sql.h"

QString SelectFromTable::subscriptionQuery = "SELECT * FROM {tenant}.%1 %2 WHERE %3 = %4";

SelectFromTable::SelectFromTable()
    : ui(new Ui::SelectFromTable)
{
}

SelectFromTable::~SelectFromTable()
{
    if (ui)
        delete ui;
}

void SelectFromTable::execute()
{
    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

        new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, true);

        m_uiSetup = true;

        connectObjects();
    }

    if(languageChanged())
    {
        ui->retranslateUi(this);
        setLanguageChanged(false);
    }

    auto stylesheet = getData()["stylesheet"].toString();
    QWidget::setStyleSheet(stylesheet);

    resetTableWidget();

    ui->labelTableNameValue->setText({});
    ui->labelCountValue    ->setText({});

    auto title   = getData()["title"       ].toString();
    auto tenant  = getData()["tenant"      ].toString();
    auto value   = getData()["value"       ].toString();
    auto table   = getData()["tableName"   ].toString();
    auto acronym = getData()["tableAcronym"].toString();
    auto profile = getData()["profile"     ].toString();

    auto allConditions = Globals::getAllProfileConditionsMap(profile);

    auto field = allConditions[title];
    if (!field.isEmpty())
    {
        auto query = subscriptionQuery.arg(table, acronym, field, "'" + value + "'");
        QTimer::singleShot(0, this, [this, tenant, query](){
            m_sqlQueryRequestMethod(tenant, query, reinterpret_cast<QObject*>(this));
        });
    }

    exec();
}

void SelectFromTable::populateTableWidget()
{
    resetTableWidget();

    if (m_sqlResult.count() <= 0)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
        populateTableWidgetHorizontal();
    else
        populateTableWidgetVertical();

    ui->pushButtonExport->setEnabled(ui->tableWidget->rowCount() > 0);

    QApplication::restoreOverrideCursor();
}

void SelectFromTable::populateTableWidgetVertical()
{
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels({tr("Campo"), tr("Valore")});

    int index = 1;
    for(auto& record : m_sqlResult)
    { // TODO: move constants somewhere else
        auto alternateColor = index % 2 == 0 ?
                    QColor::fromRgb(160, 160, 160) :
                    QColor::fromRgb(210, 210, 210);
        ++index;

        for(auto i = record.begin(); i != record.end(); ++i)
        {
            ui->tableWidget->insertRow(ui->tableWidget->rowCount());

            auto itemKey = new QTableWidgetItem(i.key());
            itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
            itemKey->setBackgroundColor(alternateColor);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, itemKey);

            auto itemValue = new QTableWidgetItem(i.value());
            itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
            itemValue->setBackgroundColor(alternateColor);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, itemValue);
        }
    }
}

void SelectFromTable::populateTableWidgetHorizontal()
{
    auto colTotal = m_sqlResult.at(0).count();
    ui->tableWidget->setColumnCount(colTotal);

    for(auto& record : m_sqlResult)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        int col = 0;
        for(auto i = record.begin(); i != record.end(); ++i)
        {
            auto itemValue = new QTableWidgetItem(i.value());
            itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
            ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, col++, itemValue);
        }
    }

    QStringList header;
    int col = 0;
    auto record = m_sqlResult.at(0);
    for(auto i = record.begin(); i != record.end(); ++i, ++col)
        header.append(i.key());
    ui->tableWidget->setHorizontalHeaderLabels(header);
}

void SelectFromTable::resetTableWidget()
{
    while (ui->tableWidget->rowCount() > 0)
        ui->tableWidget->removeRow(0);

    ui->tableWidget->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QApplication::processEvents();
}

bool SelectFromTable::isEnabled()
{
    auto title = getData()["title"].toString();
    auto value = getData()["value"].toString();
    auto table = getData()["tableName"].toString();
    return title.isEmpty() == false && value.isEmpty() == false && table.isEmpty() == false;
}

void SelectFromTable::pushButtonClose_clicked()
{
    close();
}

void SelectFromTable::pushButtonExport_clicked()
{
    m_exportResultMethod(this, ui->tableWidget, VM_HORIZONTAL);
}

void SelectFromTable::pushButtonSwitchView_clicked()
{
    if (m_currentViewMode == ViewMode::VM_VERTICAL)
    {
        m_currentViewMode = ViewMode::VM_HORIZONTAL;
        ui->pushButtonSwitchView->setText("Vista Verticale");
    }
    else
    {
        m_currentViewMode = ViewMode::VM_VERTICAL;
        ui->pushButtonSwitchView->setText("Vista Orizzontale");
    }
    populateTableWidget();
}

void SelectFromTable::sqlResult(SqlResultType result)
{
    m_sqlResult = result;
    if (m_sqlResult.isEmpty())
    {
        m_messageMethod("Nessun risultato");
    }
    else
    {
        ui->labelTenantValue   ->setText(getData()["tenant"].toString());
        ui->labelTableNameValue->setText(getData()["tableName"].toString());
        ui->labelCountValue    ->setText(QString::number(m_sqlResult.size()));

        populateTableWidget();
    }
}

void SelectFromTable::connectObjects()
{
    connect(ui->pushButtonClose     , &QPushButton::clicked, this, &SelectFromTable::pushButtonClose_clicked     );
    connect(ui->pushButtonExport    , &QPushButton::clicked, this, &SelectFromTable::pushButtonExport_clicked    );
    connect(ui->pushButtonSwitchView, &QPushButton::clicked, this, &SelectFromTable::pushButtonSwitchView_clicked);

    connect(ui->tableWidget, &TableWidget::contextMenuRequest, this, [this](QJsonObject o){
        m_contextMenuMethod(this, o);
    });
}
