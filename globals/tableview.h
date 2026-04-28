#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#if defined GLOBALSLIB_LIBRARY
    #define GLOBALS_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GLOBALS_LIB_EXPORT Q_DECL_IMPORT
#endif

#include "globals.h"

#include <QTableView>
#include <QHeaderView>

class GLOBALS_LIB_EXPORT TableView: public QTableView
{
    Q_OBJECT

public:
    TableView(QWidget* parent = nullptr);

    ~TableView();

    void setViewMode      (ViewMode vm)               { m_currentViewMode = vm;  }
    void setTenantQueryMap(const TenantQueryMap& map) { m_tenantQueryMap  = map; }

signals:
    void contextMenuRequest(QJsonObject);

private:
    void customContextMenuRequested       (const QPoint& pos);
    void header_customContextMenuRequested(const QPoint& pos);

    ViewMode       m_currentViewMode { ViewMode::VM_HORIZONTAL };
    TenantQueryMap m_tenantQueryMap  {};

    QStringList getValuesByHeaderClick  (QPoint pos) const;
    QStringList getValuesByClick        (QPoint pos) const;
    QString     getTitle                (QPoint pos) const;
    QString     getTenant               (QString title, int row)    const;
};

#endif // TABLEVIEW_H
