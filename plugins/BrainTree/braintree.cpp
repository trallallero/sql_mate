#include "braintree.h"
#include "ui_braintree.h"

#include "widgetsizetool.h"
#include "globals.h"

#include <QStringList>
#include <QProcess>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTableWidgetItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

QString BrainTree::subscriptionQuery = "\
SELECT \
    ys.code AS \"Code\" \
FROM  \
           %1.spree_orders       so  \
INNER JOIN %1.spree_line_items   sli ON sli.order_id    = so.id \
INNER JOIN %1.spree_users        su  ON su.id           = so.user_id  \
INNER JOIN %1.yolo_subscriptions ys  ON ys.insurance_id = sli.id \
WHERE  \
    su.email = '%2' \
ORDER BY ys.state \
LIMIT 1 \
";

BrainTree::BrainTree()
    : ui(new Ui::BrainTree)
{
}

BrainTree::~BrainTree()
{
    if (ui)
        delete ui;
}

void BrainTree::execute()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, true);

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

    loadConfig();

    if(hasConnectionData())
        executePython();
    else
        ui->tabWidget->setCurrentWidget(ui->tabConnections);

    QApplication::restoreOverrideCursor();
    exec();
}

bool BrainTree::isEnabled()
{
    auto title = getData()["title"].toString().toLower();
    auto value = getData()["value"].toString();
    auto isMultiValue = value.split(Globals::valuesSeparator()).count() > 1;
    return (title.contains("sottoscrizione") || title.contains("subscription") || value.contains("@"))
        && value.isEmpty() == false
        && isMultiValue == false;
}

void BrainTree::resetTableWidget()
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

ValueType BrainTree::valueType(QString value) const
{
    if (value.contains("@"))
        return ValueType::VT_EMAIL;
    return ValueType::VT_SUBSCRIPTION;
}

QStringList BrainTree::getSubscriptionsByEmail(QString email) const
{
    QStringList codes;
    auto sql = sqlDB();
    Q_ASSERT(sql);

    auto tenant = getData()["tenant"].toString();

    auto query = subscriptionQuery.arg(tenant, email);
    auto result = sql->exec(query);
    while(result.next())
    {
        auto record = result.record();
        auto fieldName = record.fieldName(0);
        codes.append(record.value(fieldName).toString());
    }
    return codes;
 }

bool BrainTree::hasConnectionData() const
{
    return (   ui->lineEditEnvironment->text().trimmed().isEmpty() == false
            && ui->lineEditMerchantId ->text().trimmed().isEmpty() == false
            && ui->lineEditPublicKey  ->text().trimmed().isEmpty() == false
            && ui->lineEditPrivateKey ->text().trimmed().isEmpty() == false);
}

void BrainTree::executePython()
{
    resetTableWidget();

    auto value = getData()["value"].toString();

    if (valueType(value) == ValueType::VT_EMAIL)
    {
        auto values = getSubscriptionsByEmail(value);
        if (values.count() > 0)
            value = values[0];
    }

    QStringList arguments {
        "plugins/braintree/braintree_fetcher.py",
        "-e", ui->lineEditEnvironment->text().trimmed(),
        "-m", ui->lineEditMerchantId ->text().trimmed(),
        "-p", ui->lineEditPublicKey  ->text().trimmed(),
        "-k", ui->lineEditPrivateKey ->text().trimmed(),
        "-s", value
    };
    QProcess p;
    p.start("python", arguments);
    p.waitForFinished();
    QByteArray error  = p.readAllStandardError();
    if(error.isEmpty() == false)
    {
        ui->textBrowserError->setVisible(true);
        ui->textBrowserError->setText(error);
        ui->tableWidget->setVisible(false);
        m_messageMethod(error);
        return;
    }
    ui->textBrowserError->setVisible(false);
    ui->tableWidget->setVisible(true);

    QByteArray result = p.readAllStandardOutput();

    auto jsonDoc   = QJsonDocument::fromJson(result);
    auto jsonArray = jsonDoc.array();
    auto tableWidgetInitialized = false;
    for (auto iter = jsonArray.begin(); iter != jsonArray.end(); ++iter)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        if (tableWidgetInitialized == false)
        {
            tableWidgetInitialized = true;
            auto columns = iter->toObject().keys().size();
            ui->tableWidget->setColumnCount(columns);
            ui->tableWidget->setHorizontalHeaderLabels({iter->toObject().keys()});
        }
        auto object = iter->toObject();
        auto column = 0;
        for (auto& key : object.keys())
        {
            auto val = object[key];
            auto item = new QTableWidgetItem(val.toString());
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setToolTip(val.toString());
            ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, column++, item);
        }
    }
}

void BrainTree::saveConfig()
{
    QJsonObject jo {
        {"environment", ui->lineEditEnvironment->text().trimmed()},
        {"merchantId" , ui->lineEditMerchantId ->text().trimmed()},
        {"publicKey"  , ui->lineEditPublicKey  ->text().trimmed()},
        {"privateKey" , Globals::getCrypted(ui->lineEditPrivateKey ->text().trimmed())},
    };
    QFile file("plugins/braintree/config.json");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QJsonDocument(jo).toJson());
}

void BrainTree::loadConfig()
{
    QFile file("plugins/braintree/config.json");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto jsonConfigDoc  = QJsonDocument::fromJson(file.readAll());
        auto jsonConfigObj  = jsonConfigDoc.object();
        file.close();

        ui->lineEditEnvironment->setText(jsonConfigObj["environment"].toString());
        ui->lineEditMerchantId ->setText(jsonConfigObj["merchantId" ].toString());
        ui->lineEditPublicKey  ->setText(jsonConfigObj["publicKey"  ].toString());
        ui->lineEditPrivateKey ->setText(Globals::getDecrypted(jsonConfigObj["privateKey" ].toString()));
    }
}

void BrainTree::pushButtonClose_clicked()
{
    close();
}

void BrainTree::pushButtonExport_clicked()
{
    m_exportResultMethod(this, ui->tableWidget, VM_HORIZONTAL);
}

void BrainTree::pushButtonSave_clicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    saveConfig();
    ui->tabWidget->setCurrentWidget(ui->tabTransactions);
    executePython();
    QApplication::restoreOverrideCursor();
}

void BrainTree::connectObjects()
{
    connect(ui->pushButtonClose , &QPushButton::clicked, this, &BrainTree::pushButtonClose_clicked );
    connect(ui->pushButtonExport, &QPushButton::clicked, this, &BrainTree::pushButtonExport_clicked);
    connect(ui->pushButtonSave  , &QPushButton::clicked, this, &BrainTree::pushButtonSave_clicked  );
}
