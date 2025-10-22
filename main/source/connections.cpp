#include "connections.h"
#include "ui_connections.h"

#include "widgetsizetool.h"
#include "globals.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlDatabase>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDebug>

QJsonArray Connections::m_connections {};

Connections::Connections(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Connections)
{
    m_dialog = Globals::createShadowedGui(this);
    ui->setupUi(m_dialog);

    new WidgetSizeTool(this, ui->groupBoxZeroMainButtons);

    QSqlDatabase::addDatabase("QPSQL", "test_connection");

    connectObjects();
    loadConnections();
    showConnections();
}

Connections::~Connections()
{
    QSqlDatabase::removeDatabase("test_connection");
    delete ui;
}

QStringList Connections::getConnectionNames()
{
    QStringList connectionNames;

    loadConnections();
    for(auto&& conn : qAsConst(m_connections))
    {
        QJsonObject element = conn.toObject();
        connectionNames.append(element["name"].toString().trimmed());
    }
    return connectionNames;
}

ConnectionValues Connections::getConnectionValues(QString name)
{
    ConnectionValues cv;
    for(auto&& conn : qAsConst(m_connections))
    {
        QJsonObject element = conn.toObject();
        if (element["name"].toString().trimmed() == name.trimmed())
        {
            cv.name      = element["name"].toString().trimmed();
            cv.host      = element["host"].toString().trimmed();
            cv.db        = element["db"  ].toString().trimmed();
            cv.user      = element["user"].toString().trimmed();
            cv.pwd       = Globals::getDecrypted(element["pwd" ].toString().trimmed());
            cv.isMultiDb = element["isMultiDatabase"].toBool();
            return cv;
        }
    }
    return cv;
}

void Connections::pushButtonClose_clicked()
{
    close();
}

void Connections::pushButtonNew_clicked()
{
    QJsonObject jo {
        {"name", Globals::newConnectionString()},
        {"host", ""},
        {"db"  , ""},
        {"user", ""},
        {"pwd" , ""}
    };
    m_connections.push_back(jo);

    m_currentItem = new QListWidgetItem(Globals::newConnectionString(), ui->listWidgetConnections);
    m_currentItem->setSelected(true);
    m_currentItemText = Globals::newConnectionString();

    clearValues();
    enableValues(true);

    ui->lineEditName->setText(Globals::newConnectionString());
    ui->lineEditName->selectAll();
    ui->lineEditName->setFocus();

    ui->listWidgetConnections->setEnabled(false);
    ui->pushButtonNew        ->setEnabled(false);
    ui->pushButtonEdit       ->setEnabled(false);
    ui->pushButtonDelete     ->setEnabled(false);
    ui->pushButtonTest       ->setEnabled(false);
    ui->pushButtonSave       ->setEnabled(true);
    ui->pushButtonCancel     ->setEnabled(true);
}

void Connections::pushButtonSave_clicked()
{
    int row = ui->listWidgetConnections->row(m_currentItem);
    for(int i = 0; i < ui->listWidgetConnections->count(); ++i)
    {
        if (i != row && ui->listWidgetConnections->item(i)->text().trimmed() == ui->lineEditName->text().trimmed())
        {
            emit showMessage(tr("Nome già in uso. Usare un altro nome"));
            ui->lineEditName->setFocus();
            return;
        }
    }

    QJsonObject jo {
        {"name"           , ui->lineEditName    ->text().trimmed()},
        {"host"           , ui->lineEditHost    ->text().trimmed()},
        {"db"             , ui->lineEditDatabase->text().trimmed()},
        {"user"           , ui->lineEditUser    ->text().trimmed()},
        {"isMultiDatabase", ui->checkBoxIsMultiDatabase->isChecked()},
        {"pwd"            , Globals::getCrypted(ui->lineEditPassword->text().trimmed())}
    };

    if (m_connections.count() == 0)
    {
        m_connections.push_back(jo);
    }
    else
    {
        int index = 0;
        for(auto&& conn : qAsConst(m_connections))
        {
            QJsonObject element = conn.toObject();
            if (element["name"].toString().trimmed() == m_currentItemText)
            {
                m_connections.replace(index, jo);
                break;
            }
            ++index;
        }
    }

    QFile file(Globals::filenameConfigConnections());
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(QJsonDocument(m_connections).toJson());

    enableValues(false);

    ui->listWidgetConnections->setEnabled(true);
    ui->pushButtonNew        ->setEnabled(true);
    ui->pushButtonDelete     ->setEnabled(true);
    ui->pushButtonTest       ->setEnabled(true);
    ui->pushButtonEdit       ->setEnabled(true);
    ui->pushButtonSave       ->setEnabled(false);
    ui->pushButtonCancel     ->setEnabled(false);

    m_isEditing
            ? emit connectionModified(m_currentItemText, ui->lineEditName->text().trimmed())
            : emit connectionAdded(ui->lineEditName->text().trimmed());
    m_currentItemText = ui->lineEditName->text().trimmed();
    m_isEditing = false;
}

void Connections::pushButtonTest_clicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto db = QSqlDatabase::database("test_connection", false);

    db.setHostName    (ui->lineEditHost    ->text().trimmed());
    db.setDatabaseName(ui->lineEditDatabase->text().trimmed());
    db.setUserName    (ui->lineEditUser    ->text().trimmed());
    db.setPassword    (ui->lineEditPassword->text().trimmed());

    if (db.isValid() == false)
    {
        ui->labelError->setText(db.lastError().driverText());
        ui->toolButtonCopyError->setEnabled(true);
    }
    else if (db.open() == false)
    {
        ui->labelError->setText(db.lastError().databaseText());
        ui->toolButtonCopyError->setEnabled(true);
    }
    else
    {
        emit showMessage(tr("Connessione avvenuta con successo!"));
        ui->labelError->setText({});
        ui->toolButtonCopyError->setEnabled(false);
        db.close();
    }
    QApplication::restoreOverrideCursor();
}

void Connections::pushButtonEdit_clicked()
{
    enableValues(true);
    m_isEditing = true;
    ui->pushButtonSave  ->setEnabled(true);
    ui->pushButtonCancel->setEnabled(false);
    ui->pushButtonEdit  ->setEnabled(false);
    ui->pushButtonDelete->setEnabled(false);
    ui->pushButtonTest  ->setEnabled(false);
    ui->pushButtonNew   ->setEnabled(false);
}

void Connections::pushButtonDelete_clicked()
{
    int index = 0;
    for(auto&& conn : qAsConst(m_connections))
    {
        QJsonObject element = conn.toObject();
        if (element["name"].toString() == m_currentItemText)
        {
            m_connections.removeAt(index);
            break;
        }
        ++index;
    }
    pushButtonSave_clicked();

    removeCurrentItem();
}

void Connections::pushButtonCancel_clicked()
{
    clearValues();
    enableValues(false);

    ui->pushButtonSave       ->setEnabled(false);
    ui->pushButtonEdit       ->setEnabled(false);
    ui->pushButtonDelete     ->setEnabled(false);
    ui->pushButtonTest       ->setEnabled(false);
    ui->pushButtonCancel     ->setEnabled(false);
    ui->pushButtonNew        ->setEnabled(true);
    ui->listWidgetConnections->setEnabled(true);

    removeCurrentItem();
}

void Connections::loadConnections()
{
    if (m_connections.count() == 0)
    {
        QFile file(Globals::filenameConfigConnections());
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        m_connections = QJsonDocument::fromJson(file.readAll()).array();
    }
}

void Connections::connectObjects()
{
    connect(ui->listWidgetConnections, &QListWidget::itemClicked, m_dialog, [this](){
        ui->labelError->setText({});
        ui->toolButtonCopyError->setEnabled(false);
    });

    connect(ui->pushButtonClose , &QPushButton::clicked, this, &Connections::pushButtonClose_clicked );
    connect(ui->pushButtonNew   , &QPushButton::clicked, this, &Connections::pushButtonNew_clicked   );
    connect(ui->pushButtonSave  , &QPushButton::clicked, this, &Connections::pushButtonSave_clicked  );
    connect(ui->pushButtonTest  , &QPushButton::clicked, this, &Connections::pushButtonTest_clicked  );
    connect(ui->pushButtonEdit  , &QPushButton::clicked, this, &Connections::pushButtonEdit_clicked  );
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &Connections::pushButtonDelete_clicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &Connections::pushButtonCancel_clicked);

    connect(ui->lineEditName         , &QLineEdit::textChanged  , this, &Connections::lineEditName_textChanged);
    connect(ui->listWidgetConnections, &QListWidget::itemClicked, this, &Connections::listWidgetConnections_itemClicked);
    connect(ui->toolButtonCopyError  , &QToolButton::clicked    , this, &Connections::toolButtonCopyError_clicked);
}

void Connections::showConnections()
{
    for(auto&& conn : qAsConst(m_connections))
    {
        QJsonObject element = conn.toObject();
        new QListWidgetItem(element["name"].toString(), ui->listWidgetConnections);
    }
}

void Connections::removeCurrentItem()
{
    ui->listWidgetConnections->takeItem(ui->listWidgetConnections->row(m_currentItem));
    if (ui->listWidgetConnections->count() > 0)
    {
        ui->listWidgetConnections->item(0)->setSelected(true);
        listWidgetConnections_itemClicked(ui->listWidgetConnections->item(0));
    }
}

void Connections::clearValues()
{
    ui->lineEditName    ->clear();
    ui->lineEditHost    ->clear();
    ui->lineEditDatabase->clear();
    ui->lineEditUser    ->clear();
    ui->lineEditPassword->clear();

    ui->checkBoxIsMultiDatabase->setChecked(false);
}

void Connections::enableValues(bool enable)
{
    ui->lineEditName    ->setEnabled(enable);
    ui->lineEditHost    ->setEnabled(enable);
    ui->lineEditDatabase->setEnabled(enable);
    ui->lineEditUser    ->setEnabled(enable);
    ui->lineEditPassword->setEnabled(enable);
    ui->checkBoxIsMultiDatabase->setEnabled(enable);

    if(enable)
        ui->lineEditName->setFocus();
}

void Connections::lineEditName_textChanged(const QString& arg1)
{
    if (m_currentItem)
        m_currentItem->setText(arg1);
}

void Connections::listWidgetConnections_itemClicked(QListWidgetItem* item)
{
    m_currentItem = item;
    m_currentItemText = item->text();

    for(auto&& conn : qAsConst(m_connections))
    {
        QJsonObject element = conn.toObject();
        if (element["name"].toString() == m_currentItemText)
        {
            ui->lineEditName    ->setText(element["name"].toString().trimmed());
            ui->lineEditHost    ->setText(element["host"].toString().trimmed());
            ui->lineEditDatabase->setText(element["db"  ].toString().trimmed());
            ui->lineEditUser    ->setText(element["user"].toString().trimmed());
            ui->lineEditPassword->setText(Globals::getDecrypted(element["pwd" ].toString().trimmed()));

            ui->checkBoxIsMultiDatabase->setChecked(element["isMultiDatabase"].toBool());

            enableValues(false);

            ui->pushButtonEdit  ->setEnabled(true);
            ui->pushButtonDelete->setEnabled(true);
            ui->pushButtonTest  ->setEnabled(true);
            ui->pushButtonNew   ->setEnabled(true);
            ui->pushButtonSave  ->setEnabled(false);
            break;
        }
    }
}

void Connections::toolButtonCopyError_clicked()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->labelError->text());
    emit showMessage(tr("Errore copiato"));
}
