#include <QFile>

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

    m_db.setConnectOptions(QString("connect_timeout=%1").arg(Globals::sqlConnectTimeout()));

    if (m_db.open() == false)
        return m_db.lastError().databaseText();
    return {};
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

void Sql::getResult(QStringList tenants, QString conditions, QString limit)
{
    createConnectionsForMultiDb(tenants);

    auto query = m_query;
    if(limit.isEmpty() == false)
        query.append(QString(" LIMIT %1").arg(limit));

    m_workerThread = new WorkerThread(qobject_cast<QObject*>(this), m_tenantDbMap, conditions, m_db, query);

    connect(m_workerThread, &WorkerThread::resultReady, this, [this](const SqlResultType& result, const TenantQueryMap& tenantQueryMap){
        if(m_fetching)
        {
            m_fetching = false;
            emit resultReady(result, tenantQueryMap);
        }
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
    connect(m_workerThread, &WorkerThread::sqlAborted, this, [this](const SqlResultType& result){
        m_fetching = false;
        emit sqlAborted(result);
    });
    connect(m_workerThread, &WorkerThread::finished, m_workerThread, &QObject::deleteLater);
    m_fetching = true;
    m_workerThread->start();
}

void Sql::getResultForPlugins(QString tenant, QString query, QObject* sender)
{
    disconnect(this, SIGNAL(sqlResult(SqlResultType)), sender, SLOT(sqlResult(SqlResultType)));
    connect   (this, SIGNAL(sqlResult(SqlResultType)), sender, SLOT(sqlResult(SqlResultType)));

    createConnectionsForMultiDb({tenant});

    auto queryStr = query.replace("{tenant}", tenant);
    auto result   = m_tenantDbMap[tenant].exec(queryStr);

    if (result.isValid() == false && result.lastError().databaseText().isEmpty() == false)
    {
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
    emit sqlResult(records);
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
