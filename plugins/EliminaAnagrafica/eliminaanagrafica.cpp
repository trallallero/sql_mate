#include "eliminaanagrafica.h"
#include "ui_eliminaanagrafica.h"
#include "queries.h"
#include "widgetsizetool.h"

#include <exception>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QCheckBox>
#include <QDebug>
//#include <QDesktopServices>

EliminaAnagrafica::EliminaAnagrafica(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EliminaAnagrafica)
    , m_configFilename(QString("%1/plugins/eliminaanagrafica/config.json").arg(QDir::currentPath()))
{
}

EliminaAnagrafica::~EliminaAnagrafica()
{
    if (ui)
        delete ui;
    ui = nullptr;
}

void EliminaAnagrafica::execute()
{
    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, true);

        connectObjects();

        m_uiSetup = true;
    }

    cleanUp();

    m_mainwidgetConnectionName = connectionsName();
    loadAndApplyConfig();

    if(languageChanged())
    {
        ui->retranslateUi(this);
        setLanguageChanged(false);
    }

    auto stylesheet = getData()["stylesheet"].toString();
    QWidget::setStyleSheet(stylesheet);

    ui->tableWidgetOrdersSolidus->setToolTip(ui->tableWidgetOrdersSolidus->styleSheet());

    exec();
}

void EliminaAnagrafica::saveConfig()
{
    QJsonObject jo{
        {"yinConnection"    , ui->comboBoxConnectionYIN->currentText()    },
        {"solidusConnection", ui->comboBoxConnectionSolidus->currentText()}
    };

    QFile file("plugins/EliminaAnagrafica/config.json");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QJsonDocument(jo).toJson());
    file.close();
    m_messageMethod(tr("Configurazione salvata"));
}

void EliminaAnagrafica::findValues()
{
    setConnection(CT_YIN);

    if(ui->lineEditEmail->text().isEmpty())
    {
        bool error;
        auto email = yinSelectEmail(&error);
        if(error)
        {
            m_messageMethod(tr("Nessun record è stato trovato!"));
            return;
        }
        if(email.isEmpty())
            return;
        ui->lineEditEmail->setText(email);
    }

    yinSelectUserId();
    if(m_yinUserId > 0)
    {
        yinGetValuesByEmail();
        yinSelectOrders();
    }

    setConnection(CT_SOLIDUS);

    solidusSelectUserId();
    if(m_solidusUserId > 0)
    {
        solidusGetValuesByEmail();
        solidusSelectOrders();
    }

    ui->pushButtonRemove       ->setEnabled(m_yinUserId > 0 || m_solidusUserId > 0);
    ui->pushButtonCreateQueries->setEnabled(m_yinUserId > 0 || m_solidusUserId > 0);
}

void EliminaAnagrafica::remove()
{
    try
    {
        m_deleteQueriesViewer.clear();

        if(m_yinUserId > 0)
        {
            setConnection(CT_YIN);
            yinDeleteCustomerSeller();
            yinDeleteCustomer();
            if(ui->checkBoxRemoveOrders->isChecked())
                yinDeleteOrders();
        }

        if(m_solidusUserId > 0)
        {
            setConnection(CT_SOLIDUS);
            solidusDeleteYoloUserAcceptances();
            solidusDeleteSpreeAddressesShip();
            solidusDeleteSpreeAddressesBill();
            solidusDeleteSpreeUsers();

            if(ui->checkBoxRemoveOrders->isChecked())
            {
                solidusDeleteYoloInsuranceInfos();
                solidusDeleteSpreeLineItems();
                solidusDeleteSpreeOrders();
            }
        }
    }
    catch(...)
    {
        m_sql->rollback();
    }
}

void EliminaAnagrafica::enableSearchButton()
{
    ui->pushButtonFind->setEnabled(
        (ui->lineEditName   ->text().isEmpty() == false && // either name + surname
         ui->lineEditSurname->text().isEmpty() == false &&
         ui->lineEditEmail  ->text().isEmpty())
        ||
        (ui->lineEditName   ->text().isEmpty() &&          // or email
         ui->lineEditSurname->text().isEmpty() &&
         ui->lineEditEmail  ->text().isEmpty() == false)
    );
}

void EliminaAnagrafica::loadAndApplyConfig()
{
    QFile file("plugins/EliminaAnagrafica/config.json");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto jsonConfigDoc      = QJsonDocument::fromJson(file.readAll());
        auto jsonConfigObj      = jsonConfigDoc.object();
        m_yinConnectionName     = jsonConfigObj["yinConnection"    ].toString();
        m_solidusConnectionName = jsonConfigObj["solidusConnection"].toString();
        file.close();
    }
    addConnectionItems();
    setTenants();
}

void EliminaAnagrafica::setTenants()
{
    for(auto& item : std::map<QComboBox*, std::pair<QComboBox*, QString> >{
        {ui->comboBoxConnectionSolidus, {ui->comboBoxTenantSolidus, tenantsQuerySolidus}},
        {ui->comboBoxConnectionYIN    , {ui->comboBoxTenantYIN    , tenantsQueryYin    }}
    })
    {
        item.second.first->clear();

        QList<QString>  tenants;
        m_sqlConnectionMethod(item.first->currentText(), false);
        m_sql = sqlDB();
        auto query = m_sql->exec(item.second.second);
        while(query.next())
            tenants.append(query.value(0).toString());
        std::sort(tenants.begin(), tenants.end());
        item.second.first->addItems(tenants);
    }
}

QString EliminaAnagrafica::yinSelectEmail(bool* error)
{
    *error = false;

    auto q = yin_get_email_by_name
            .replace("{tenant}"  , ui->comboBoxTenantYIN->currentText())
            .replace("{name}"    , ui->lineEditName->text())
            .replace("{surname}" , ui->lineEditSurname->text());

    qDebug() << ui->lineEditSurname->text();

    auto result = m_sql->exec(q);
    QStringList emails;
    while(result.next())
    {
        auto record = result.record();
        if(result.size() > 1)
            emails.append(record.value(0).toString());
        else
            return record.value(0).toString();
    }
    if(result.size() > 1)
    {
        m_deleteQueriesViewer.clear();
        m_deleteQueriesViewer.appendText(tr("Trovata più di una email:\n"));
        for(auto& email : emails)
            m_deleteQueriesViewer.appendText(email);
        m_deleteQueriesViewer.exec();
    }
    else
    {
        *error = true; // not found
    }
    return {};
}

void EliminaAnagrafica::yinSelectUserId()
{
    m_yinUserId = 0;

    auto q = yin_select_user_id
            .replace("{tenant}", ui->comboBoxTenantYIN->currentText())
            .replace("{email}" , ui->lineEditEmail->text());

    auto result = m_sql->exec(q);
    while(result.next())
    {
        auto record = result.record();
        m_yinUserId = record.value(0).toInt();
    }
    if(result.size() <= 0)
        m_messageMethod(tr("Nessun record trovato su YIN"));
}

void EliminaAnagrafica::yinGetValuesByEmail()
{
    auto q = yin_get_values_by_email
            .replace("{tenant}", ui->comboBoxTenantYIN->currentText())
            .replace("{email}" , ui->lineEditEmail->text());

    auto result = m_sql->exec(q);
    while(result.next())
    {
        auto record = result.record();
        ui->labelShowNameYIN   ->setText(record.value(0).toString());
        ui->labelShowSurnameYIN->setText(record.value(1).toString());
    }
}

void EliminaAnagrafica::yinSelectOrders()
{
    auto q = yin_select_orders
            .replace("{tenant}"     , ui->comboBoxTenantYIN->currentText())
            .replace("{yin_user_id}", QString::number(m_yinUserId));

    auto result = m_sql->exec(q);
    int count = 0;
    while(result.next())
    {
        ui->tableWidgetOrdersYIN->insertRow(ui->tableWidgetOrdersYIN->rowCount());

        auto record = result.record();

        auto item = new QTableWidgetItem(record.value(0).toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableWidgetOrdersYIN->setItem(ui->tableWidgetOrdersYIN->rowCount() - 1, 0, item);
        ++count;
    }
    if(count > 0)
    {
        ui->tableWidgetOrdersYIN    ->setHorizontalHeaderLabels({tr("Ordine")});
        ui->tableWidgetOrdersYIN->horizontalHeader()->show();
    }
}

void EliminaAnagrafica::yinDeleteCustomerSeller()
{
    auto q = yin_delete_customer_seller
            .replace("{tenant}"     , ui->comboBoxTenantYIN->currentText())
            .replace("{yin_user_id}", QString::number(m_yinUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::yinDeleteCustomer()
{
    auto q = yin_delete_customer
            .replace("{tenant}"     , ui->comboBoxTenantYIN->currentText())
            .replace("{yin_user_id}", QString::number(m_yinUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::yinDeleteOrders()
{
    auto q = yin_delete_orders
            .replace("{tenant}"     , ui->comboBoxTenantYIN->currentText())
            .replace("{yin_user_id}", QString::number(m_yinUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::solidusSelectUserId()
{
    m_solidusUserId = 0;

    auto q = solidus_select_user_id
            .replace("{tenant}", ui->comboBoxTenantSolidus->currentText())
            .replace("{email}" , ui->lineEditEmail->text());

    auto result = m_sql->exec(q);
    while(result.next())
    {
        auto record = result.record();
        m_solidusUserId = record.value(0).toInt();
    }
    if(result.size() <= 0)
        m_messageMethod(tr("Nessun record trovato su SOLIDUS"));
}

void EliminaAnagrafica::solidusGetValuesByEmail()
{
    auto q = solidus_get_values_by_email
            .replace("{tenant}", ui->comboBoxTenantSolidus->currentText())
            .replace("{email}" , ui->lineEditEmail->text());

    auto result = m_sql->exec(q);
    while(result.next())
    {
        auto record = result.record();
        ui->labelShowNameSolidus   ->setText(record.value(0).toString());
        ui->labelShowSurnameSolidus->setText(record.value(1).toString());
    }
}

void EliminaAnagrafica::solidusSelectOrders()
{
    auto q = solidus_select_orders
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));

    auto result = m_sql->exec(q);
    int count = 0;
    while(result.next())
    {
        ui->tableWidgetOrdersSolidus->insertRow(ui->tableWidgetOrdersSolidus->rowCount());

        auto record = result.record();

        auto item = new QTableWidgetItem(record.value(0).toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableWidgetOrdersSolidus->setItem(ui->tableWidgetOrdersSolidus->rowCount() - 1, 0, item);

        item = new QTableWidgetItem(record.value(1).toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableWidgetOrdersSolidus->setItem(ui->tableWidgetOrdersSolidus->rowCount() - 1, 1, item);
        ++count;
    }
    if(count > 0)
    {
        ui->tableWidgetOrdersSolidus->setHorizontalHeaderLabels({tr("Ordine"), tr("Polizza")});
        ui->tableWidgetOrdersSolidus->horizontalHeader()->show();
    }
}

void EliminaAnagrafica::solidusDeleteYoloUserAcceptances()
{
    auto q = solidus_delete_yolo_user_acceptances
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::solidusDeleteYoloInsuranceInfos()
{
    auto q = solidus_delete_yolo_insurance_infos
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::solidusDeleteSpreeLineItems()
{
    auto q = solidus_delete_spree_line_items
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::solidusDeleteSpreeOrders()
{
    auto q = solidus_delete_spree_orders
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::solidusDeleteSpreeAddressesShip()
{
    auto q = solidus_delete_spree_addresses_ship
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q, false);
}

void EliminaAnagrafica::solidusDeleteSpreeAddressesBill()
{
    auto q = solidus_delete_spree_addresses_bill
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q, false);
}

void EliminaAnagrafica::solidusDeleteSpreeUsers()
{
    auto q = solidus_delete_spree_users
            .replace("{tenant}"         , ui->comboBoxTenantSolidus->currentText())
            .replace("{solidus_user_id}", QString::number(m_solidusUserId));
    deleteFromQuery(q);
}

void EliminaAnagrafica::deleteFromQuery(QString query, bool failOnNoRowsAffected /*= true*/)
{
    if(m_onlyCreateQueries)
    {
        m_deleteQueriesViewer.appendText(query + ";");
        return;
    }
    auto result = m_sql->exec(query);
    if (failOnNoRowsAffected && result.numRowsAffected() <= 0)
    {
        auto error = m_sql->lastError().databaseText().toLocal8Bit();
        throw std::runtime_error(error);
    }
}

void EliminaAnagrafica::connectObjects()
{
    connect(&m_deleteQueriesViewer       , &DeleteQueriesViewer::message, this, EliminaAnagrafica::m_messageMethod);

    connect(ui->pushButtonCancel         , &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButtonSaveConnections, &QPushButton::clicked, this, &EliminaAnagrafica::saveConfig);
    connect(ui->pushButtonFind           , &QPushButton::clicked, this, &EliminaAnagrafica::findValues);
    connect(ui->pushButtonRemove         , &QPushButton::clicked, this, &EliminaAnagrafica::remove);
    connect(ui->pushButtonCreateQueries  , &QPushButton::clicked, this, [this](){
        m_onlyCreateQueries = true;
        remove();
        m_onlyCreateQueries = false;
        m_deleteQueriesViewer.exec();
    });

    connect(this, &QDialog::finished, this, [this](){
        if(m_sql && m_sql->isOpen())
            m_sql->close();

        m_sqlConnectionMethod(m_mainwidgetConnectionName, true);
    });
    connect(ui->lineEditName   , &QLineEdit::textChanged, this, [this](){ enableSearchButton(); });
    connect(ui->lineEditSurname, &QLineEdit::textChanged, this, [this](){ enableSearchButton(); });
    connect(ui->lineEditEmail  , &QLineEdit::textChanged, this, [this](){ enableSearchButton(); });
}

void EliminaAnagrafica::cleanUp()
{
    m_solidusUserId = m_yinUserId = 0;

    ui->lineEditName            ->setText({});
    ui->lineEditSurname         ->setText({});
    ui->lineEditEmail           ->setText({});
    ui->labelShowNameSolidus    ->setText({});
    ui->labelShowNameYIN        ->setText({});
    ui->labelShowSurnameSolidus ->setText({});
    ui->labelShowSurnameYIN     ->setText({});
    ui->checkBoxRemoveOrders    ->setChecked(false);

    resetTableWidget(ui->tableWidgetOrdersSolidus);
    resetTableWidget(ui->tableWidgetOrdersYIN);

    ui->pushButtonRemove       ->setEnabled(false);
    ui->pushButtonCreateQueries->setEnabled(false);

    ui->lineEditName->setFocus();
}

void EliminaAnagrafica::resetTableWidget(QTableWidget* w)
{
    w->clear();
    w->setRowCount(0);
    w->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    w->horizontalHeader()->hide();
}

void EliminaAnagrafica::setConnection(ConnectionType ct)
{
    m_sqlConnectionMethod(ct == CT_YIN ? ui->comboBoxConnectionYIN->currentText() : ui->comboBoxConnectionSolidus->currentText(), false);
    m_sql = sqlDB();
}

void EliminaAnagrafica::addConnectionItems()
{
    ui->comboBoxConnectionSolidus->clear();
    ui->comboBoxConnectionSolidus->addItems(connectionsNames());
    ui->comboBoxConnectionSolidus->setCurrentIndex(ui->comboBoxConnectionSolidus->findText(m_solidusConnectionName));

    ui->comboBoxConnectionYIN->clear();
    ui->comboBoxConnectionYIN->addItems(connectionsNames());
    ui->comboBoxConnectionYIN->setCurrentIndex(ui->comboBoxConnectionYIN->findText(m_yinConnectionName));
}
