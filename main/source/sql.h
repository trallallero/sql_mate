#ifndef SQL_H
#define SQL_H

#include "globals.h"
#include "connections.h"

#include <QSqlDatabase>
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QList>
#include <QThread>
#include <QDebug>
#include <functional>

using TenantDbMap = QMap<QString, QSqlDatabase>;

using ViewmodeSqlconverter = QMap<ViewMode, std::function<int(QSqlQuery&, QStandardItemModel&, int&, bool&)>>;

class ModelConverter : public QObject
{
    Q_OBJECT

public:
    static void modelToVerticalModel(QStandardItemModel& srcModel, QStandardItemModel& destModel)
    {
        destModel.clear();
        ModelConverter::setHeaders_V(destModel);

        QFont boldFont;
        boldFont.setBold(true);

        for (int row = 0, destRow = 0, index = 0; row < srcModel.rowCount(); ++row)
        {
            auto alternateColor = Globals::getAlternateColor(index);

            for (int col = 0; col < srcModel.columnCount(); ++col)
            {
                QStandardItem* item = srcModel.item(row, col);
                auto text = item->data(Qt::DisplayRole).toString();

                auto title = new QStandardItem(srcModel.headerData(col, Qt::Horizontal).toString());
                title->setFlags(title->flags() ^ Qt::ItemIsEditable);
                title->setData(alternateColor, Qt::BackgroundRole);
                title->setFont(boldFont);

                auto value = new QStandardItem(text);
                value->setFlags(value->flags() ^ Qt::ItemIsEditable);
                value->setData(alternateColor, Qt::BackgroundRole);

                destModel.setItem(destRow, 0, title);
                destModel.setItem(destRow, 1, value);
                ++destRow;
            }
        }
    }

    static void modelToHorizontalModel(
            QStandardItemModel& srcModel,
            QStandardItemModel& destModel,
            int columnCount)
    {
        auto headerSet = false;
        destModel.clear();

        for (int row = 0, col = 0, destRow = 0; row < srcModel.rowCount(); ++row)
        {
            auto alternateColor = Globals::getAlternateColor(destRow, false);

            auto title = srcModel.item(row, 0)->data(Qt::DisplayRole).toString();
            auto value = srcModel.item(row, 1)->data(Qt::DisplayRole).toString();

            if (headerSet == false)
            {
                auto item = new QStandardItem(title);
                destModel.setHorizontalHeaderItem(col, item);
            }

            auto item = new QStandardItem(value);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setData(alternateColor, Qt::BackgroundRole);
            destModel.setItem(destRow, col, item);

            ++col;
            if (col >= columnCount)
            {
                ++destRow;
                col       = 0;
                headerSet = true;
            }
        }
    }

    static int queryToHorizontalModel(QSqlQuery& query, QStandardItemModel& model, int& row, bool& headersSet)
    {
        auto index = 0, colCount = 0;
        while(query.next())
        {
            auto record = query.record();

            if (colCount == 0)
                colCount = record.count();

            auto alternateColor = Globals::getAlternateColor(index);

            if (headersSet == false)
            {
                ModelConverter::setHeaders_H(record, model);
                headersSet = true;
            }

            for (int col = 0; col < record.count(); ++col)
            {
                QString fieldName = model.headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
                auto item = new QStandardItem(query.value(fieldName).toString());
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                item->setData(alternateColor, Qt::BackgroundRole);
                model.setItem(row, col, item);
            }
            ++row;
        }
        return colCount;
    }

    static int queryToVerticalModel(QSqlQuery& query, QStandardItemModel& model, int& row, bool& headersSet)
    {
        QFont boldFont;
        boldFont.setBold(true);

        auto sortedHeaders = QStringList();

        auto index = 0, colCount = 0;
        while(query.next())
        {
            auto record = query.record();

            if (colCount == 0)
                colCount = record.count();

            if (sortedHeaders.isEmpty())
                sortedHeaders = ModelConverter::getSortedHeaders(record);

            auto alternateColor = Globals::getAlternateColor(index);

            if (headersSet == false)
            {
                ModelConverter::setHeaders_V(model);
                headersSet = true;
            }

            for (auto header : sortedHeaders)
            {
                auto title = new QStandardItem(header);
                auto value = new QStandardItem(record.value(header).toString());

                title->setFlags(title->flags() ^ Qt::ItemIsEditable);
                title->setData(alternateColor, Qt::BackgroundRole);
                title->setFont(boldFont);

                value->setFlags(title->flags() ^ Qt::ItemIsEditable);
                value->setData(alternateColor, Qt::BackgroundRole);

                model.setItem(row, 0, title);
                model.setItem(row, 1, value);
                ++row;
            }
        }
        return colCount;
    }

    static void setHeaders_H(QSqlRecord record, QStandardItemModel& model)
    {
        model.setHorizontalHeaderLabels(getSortedHeaders(record));
    }

    static void setHeaders_V(QStandardItemModel& model)
    {
        model.setHorizontalHeaderLabels({tr("Campo"), tr("Valore")});
    }

    static QStringList getSortedHeaders(QSqlRecord record)
    {
        QStringList headers;

        for (int i = 0; i < record.count(); ++i)
            headers.append(record.fieldName(i));

        headers.sort();
        return headers;
    }
};



class WorkerThread : public QThread
{
    Q_OBJECT

public:
    WorkerThread(
            QObject* parent,
            TenantDbMap tenantsDbMap,
            QString conditions,
            QSqlDatabase db,
            QString query,
            QStandardItemModel& model,
            ViewMode viewMode
        )
        : QThread(parent)
        , m_tenantDbMap(tenantsDbMap)
        , m_conditions(conditions)
        , m_db(db)
        , m_query(query)
        , m_model(model)
        , m_viewMode(viewMode)
        , m_viewmodeSqlconverter(
        {
            { ViewMode::VM_HORIZONTAL, &ModelConverter::queryToHorizontalModel},
            { ViewMode::VM_VERTICAL  , &ModelConverter::queryToVerticalModel}
        })
    {
    }

    virtual void run() override
    {
        TenantQueryMap tenantQueryMap;
        auto           headersSet = m_model.rowCount() > 0;
        auto           row        = m_model.rowCount();

        for (auto& tenant : m_tenantDbMap.keys())
        {
            emit currentTenant(tenant);

            if (isInterruptionRequested())
            {
                emit sqlAborted();
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
                emit resultReady(tenantQueryMap);
                return;
            }
            else if (query.size() > 0)
                tenantQueryMap[tenant] = queryStr;

            int colCount = m_viewmodeSqlconverter[m_viewMode](query, m_model, row, headersSet);
            emit columnCount(colCount);
        }
        emit resultReady(tenantQueryMap);
    }

signals:
    void currentTenant(const QString);
    void resultReady(const TenantQueryMap&);
    void queryExecuted(const QString);
    void sqlError(const QString);
    void sqlAborted();
    void columnCount(int);

private:

    TenantDbMap          m_tenantDbMap;
    QString              m_conditions;
    QSqlDatabase         m_db;
    QString              m_query;
    QStandardItemModel&  m_model;
    ViewMode             m_viewMode;
    ViewmodeSqlconverter m_viewmodeSqlconverter;
};

class Sql : public QObject
{
    Q_OBJECT

public:
    Sql();
    ~Sql() = default;

    QString initalize(ConnectionValues cv);

    bool updateConnection(QString oldName, QString newName);

    QList<QString> getTenants();
    void           getResult(QStringList tenants, QString conditions, QString limit, bool clear);
    void           getResultForPlugins(QString tenant, QString query, QObject* sender, bool fallbackWhenError = false);

    QSqlDatabase* sqlDB() { return &m_db; }

    QStandardItemModel& model(ViewMode mode)
    {
        m_lastMode = mode;
        if (m_lastMode == VM_HORIZONTAL)
        {
            if(m_model_H.rowCount() == 0 && m_model_V.rowCount() > 0)
                ModelConverter::modelToHorizontalModel(m_model_V, m_model_H, m_columnCount);
            return m_model_H;
        }
        else // VM_VERTICAL
        {
            if(m_model_V.rowCount() == 0 && m_model_H.rowCount() > 0)
                ModelConverter::modelToVerticalModel(m_model_H, m_model_V);
            return m_model_V;
        }
    }

    bool isFetching() const { return m_fetching; }
    void setQuery();
    void abort();

signals:
    void currentTenant(const QString);
    void resultReady(const TenantQueryMap&);
    void queryExecuted(const QString);
    void sqlError(const QString);
    void sqlAborted();
    void sqlResult(SqlResultType, int);

private:
    QSqlDatabase       m_db;
    QStandardItemModel m_model_H;
    QStandardItemModel m_model_V;
    QString            m_query;
    QString            m_tenantsQuery;
    QString            m_currentConnectionName {};
    bool               m_fetching     { false };
    bool               m_isMultiDb    { false };
    WorkerThread*      m_workerThread { nullptr };
    ViewMode           m_lastMode     { ViewMode::VM_HORIZONTAL };
    int                m_columnCount  {0};

    TenantDbMap m_tenantDbMap;

    void createConnectionsForMultiDb(QStringList tenants);
};

#endif // SQL_H
