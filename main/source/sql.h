#ifndef SQL_H
#define SQL_H

#include "globals.h"
#include "connections.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QList>
#include <QThread>
#include <QDebug>

using TenantDbMap = QMap<QString, QSqlDatabase>;

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(QObject* parent, TenantDbMap tenantsDbMap, QString conditions, QSqlDatabase db, QString query)
        : QThread(parent)
        , m_tenantDbMap(tenantsDbMap)
        , m_conditions(conditions)
        , m_db(db)
        , m_query(query)
    {
    }

    virtual void run() override
    {
        TenantQueryMap tenantQueryMap;

        SqlResultType result;
        for (auto& tenant : m_tenantDbMap.keys())
        {
            emit currentTenant(tenant);

            if (isInterruptionRequested())
            {
                emit sqlAborted(result);
                break;
            }

            m_db = m_tenantDbMap[tenant];

            if(m_db.isOpen() == false)
                m_db.open();

            auto nonConstQuery = m_query;
            auto queryStr = nonConstQuery
                    .replace("{tenant}", tenant)
                    .replace("{conditions}", m_conditions);

            auto query = m_db.exec(queryStr);

            emit queryExecuted(queryStr);

            if (query.isValid() == false && query.lastError().databaseText().isEmpty() == false)
            {
                emit sqlError(query.lastError().databaseText());
                emit resultReady({}, tenantQueryMap);
                return;
            }
            else if (query.size() > 0)
                tenantQueryMap[tenant] = queryStr;

            while(query.next())
            {
                QMap<QString, QString> record_map;
                auto record = query.record();
                for (int var = 0; var < record.count(); ++var)
                {
                    QString fieldName = record.fieldName(var);
                    record_map[fieldName] = record.value(fieldName).toString();
                }
                result.append(record_map);
            }
        }
        emit resultReady(result, tenantQueryMap);
    }
signals:
    void currentTenant(const QString);
    void resultReady(const SqlResultType&, const TenantQueryMap&);
    void queryExecuted(const QString);
    void sqlError(const QString);
    void sqlAborted(const SqlResultType&);

private:
    TenantDbMap  m_tenantDbMap;
    QString      m_conditions;
    QSqlDatabase m_db;
    QString      m_query;
};

class Sql : public QObject
{
    Q_OBJECT

public:
    Sql();
    ~Sql() = default;

    QString initalize(ConnectionValues cv);

    QList<QString> getTenants();
    void           getResult(QStringList tenants, QString conditions, QString limit);
    void           getResultForPlugins(QString tenant, QString query, QObject* sender);

    QSqlDatabase* sqlDB() { return &m_db; }

    bool isFetching() const { return m_fetching; }
    void setQuery();
    void abort();

signals:
    void currentTenant(const QString);
    void resultReady(const SqlResultType&, const TenantQueryMap&);
    void queryExecuted(const QString);
    void sqlError(const QString);
    void sqlAborted(const SqlResultType&);
    void sqlResult(SqlResultType);

private:
    QSqlDatabase    m_db;
    QString         m_query;
    QString         m_tenantsQuery;
    bool            m_fetching     { false };
    bool            m_isMultiDb    { false };
    WorkerThread*   m_workerThread { nullptr };

    TenantDbMap m_tenantDbMap;

    void createConnectionsForMultiDb(QStringList tenants);
};

#endif // SQL_H
