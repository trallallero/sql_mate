#include <QFile>
#include <QTimer>

#include "sql.h"
#include "globals.h"

Sql::Sql()
{
}

QString Sql::initalize(ConnectionValues cv)
{  
    QFile file_tenants(Globals::filenameProfileTenantsQuery());
    file_tenants.open(QIODevice::ReadOnly | QIODevice::Text);
    m_tenantsQuery = file_tenants.readAll();

    if (QSqlDatabase::connectionNames().contains(cv.name) == false)
        m_db = QSqlDatabase::addDatabase("QPSQL", cv.name);
    else
        m_db = QSqlDatabase::database(cv.name, false);

    m_db.setDatabaseName(cv.db);
    m_db.setUserName    (cv.user);
    m_db.setHostName    (cv.host);
    m_db.setPassword    (cv.pwd);

    m_isMultiDb = cv.isMultiDb;

    m_currentConnectionName = cv.name;

    m_db.setConnectOptions(QString("connect_timeout=%1").arg(Globals::sqlConnectTimeout()));

    if (m_db.open() == false)
        return m_db.lastError().databaseText();
    return {};
}

bool Sql::updateConnection(QString oldName, QString newName)
{
    if (oldName == m_currentConnectionName)
    {
        initalize(Connections::getConnectionValues(newName));
        return true;
    }
    return false;
}

QList<QString> Sql::getTenants()
{
    QList<QString>  tenants;
    auto query = m_db.exec(m_tenantsQuery);
    while(query.next())
        tenants.append(query.value(0).toString());
    std::sort(tenants.begin(), tenants.end());
    return tenants;
}

void Sql::getResult(QStringList tenants, QString conditions, QString limit, bool clear)
{
    if(clear)
    {
        m_model_H.clear();
        m_model_V.clear();
    }

    createConnectionsForMultiDb(tenants);

    auto query = m_query;
    if(limit.isEmpty() == false)
        query.append(QString(" LIMIT %1").arg(limit));

    m_workerThread = new WorkerThread(
                qobject_cast<QObject*>(this),
                m_tenantDbMap,
                conditions,
                m_db,
                query,
                m_lastMode == ViewMode::VM_HORIZONTAL ? m_model_H : m_model_V,
                m_lastMode
    );

    connect(m_workerThread, &WorkerThread::resultReady, this, [this](const TenantQueryMap& tenantQueryMap){
        if(m_fetching)
            m_fetching = false;
        emit resultReady(tenantQueryMap);
    });

    connect(m_workerThread, &WorkerThread::queryExecuted, this, [this](const QString query){
        emit queryExecuted(query);
    });
    connect(m_workerThread, &WorkerThread::currentTenant, this, [this](const QString tenant){
        emit currentTenant(tenant);
    });
    connect(m_workerThread, &WorkerThread::sqlError, this, [this](const QString error){
        m_fetching = false;
        emit sqlError(error);
    });
    connect(m_workerThread, &WorkerThread::sqlAborted, this, [this](){
        m_fetching = false;
        emit sqlAborted();
    });
    connect(m_workerThread, &WorkerThread::columnCount, this, [this](int colCount){
        m_columnCount = colCount;
    }, Qt::QueuedConnection);
    connect(m_workerThread, &WorkerThread::finished, m_workerThread, &QObject::deleteLater);

    m_fetching = true;
    m_workerThread->start();
}

void Sql::getResultForPlugins(QString tenant, QString query, QObject* sender, bool fallbackWhenError)
{
    disconnect(this, SIGNAL(sqlResult(SqlResultType,int)), sender, SLOT(sqlResult(SqlResultType,int)));
    connect   (this, SIGNAL(sqlResult(SqlResultType,int)), sender, SLOT(sqlResult(SqlResultType,int)));

    createConnectionsForMultiDb({tenant});

    auto queryCopy = query;
    auto queryStr = m_isMultiDb || fallbackWhenError
        ? queryCopy.replace("{tenant}.", "")
        : queryCopy.replace("{tenant}", tenant);

    qDebug() << queryStr;

    auto result   = m_tenantDbMap[tenant].exec(queryStr);

    if (result.isValid() == false && result.lastError().databaseText().isEmpty() == false)
    {
        if (fallbackWhenError == false)
        {
            return getResultForPlugins(tenant, query, sender, true);
        }
        qDebug() << result.lastError().databaseText();
        emit sqlError(result.lastError().databaseText());
        return;
    }

    SqlResultType records;

    while(result.next())
    {
        QMap<QString, QString> record_map;
        auto record = result.record();
        for (int var = 0; var < record.count(); ++var)
        {
            QString fieldName = record.fieldName(var);
            record_map[fieldName] = record.value(fieldName).toString();
        }
        records.append(record_map);
    }
    emit sqlResult(records, result.numRowsAffected());
    if (fallbackWhenError)
        QTimer::singleShot(1000, this, [this](){
            emit sqlError(
                tr("\nATTENZIONE: Ha fallito e poi funzionato dopo aver forzato la connessione come 'Multi database'!"
                   "\nSi raccomanda di impostare la connessione in maniera corretta!")
            );
        });
}

void Sql::setQuery()
{
    QFile file_query(Globals::filenameProfileQuery());
    file_query.open(QIODevice::ReadOnly | QIODevice::Text);
    m_query = file_query.readAll();
    file_query.close();
}

void Sql::abort()
{
    m_workerThread->requestInterruption();
}

void Sql::createConnectionsForMultiDb(QStringList tenants)
{
    m_tenantDbMap.clear();

    auto userName     = m_db.userName();
    auto hostName     = m_db.hostName();
    auto pwd          = m_db.password();

    for(auto& tenant : tenants)
    {
        if(m_isMultiDb)
        {
            auto connectionName = m_db.connectionName() + "_" + tenant;

            QSqlDatabase db;

            if (QSqlDatabase::connectionNames().contains(connectionName) == false)
                db = QSqlDatabase::addDatabase("QPSQL", connectionName);
            else
                db = QSqlDatabase::database(connectionName, false);

            db.setUserName(userName);
            db.setHostName(hostName);
            db.setPassword(pwd);
            db.setDatabaseName(tenant);
            m_tenantDbMap[tenant] = db;
        }
        else
        {
            m_tenantDbMap[tenant] = m_db;
        }
    }
}
