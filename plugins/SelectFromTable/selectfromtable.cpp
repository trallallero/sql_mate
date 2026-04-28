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

QString SelectFromTable::selectQuery = "SELECT * FROM {tenant}.%1 %2 WHERE %3 = %4";

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
        ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

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

    ui->tableWidget->resetWidget();

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
        auto query = selectQuery.arg(table, acronym, field, "'" + value + "'");
        QTimer::singleShot(0, this, [this, tenant, query](){
            m_sqlQueryRequestMethod(tenant, query, reinterpret_cast<QObject*>(this));
        });
    }

    exec();
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
    if (ui->tableWidget->viewMode() == ViewMode::VM_VERTICAL)
    {
        ui->tableWidget->setViewMode(ViewMode::VM_HORIZONTAL);
        ui->pushButtonSwitchView->setText(tr("Vista Verticale"));
    }
    else
    {
        ui->tableWidget->setViewMode(ViewMode::VM_VERTICAL);
        ui->pushButtonSwitchView->setText(tr("Vista Orizzontale"));
    }

    ui->tableWidget->populate(m_sqlResult, {});
}

void SelectFromTable::sqlResult(SqlResultType result, int rowsAffected)
{
    Q_UNUSED(rowsAffected)

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

        ui->tableWidget->populate(result, {});
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
