#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#if defined GLOBALSLIB_LIBRARY
    #define GLOBALS_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GLOBALS_LIB_EXPORT Q_DECL_IMPORT
#endif

#include "globals.h"

#include <QTableWidget>
#include <QMouseEvent>

class GLOBALS_LIB_EXPORT TableWidget : public QTableWidget
{
    Q_OBJECT

public:
    TableWidget(QWidget* parent = nullptr);
    ~TableWidget();

    void scrollTo(const QModelIndex& index, ScrollHint hint) override;

    void resetWidget();

    void filterResultFields(QStringList selectedFields);

    void populate(SqlResultType sqlResult, QStringList selectedFields);

    ViewMode viewMode() const { return m_currentViewMode; }

    void setViewMode      (ViewMode vm)               { m_currentViewMode = vm; }
    void setTenantQueryMap(const TenantQueryMap& map) { m_tenantQueryMap = map; }

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

signals:
    void visibleItemsCount(int);
    void contextMenuRequest(QJsonObject);

private:
    ViewMode       m_currentViewMode { ViewMode::VM_HORIZONTAL };
    TenantQueryMap m_tenantQueryMap  {};

    QStringList getValuesByHeaderClick  (QPoint pos) const;
    QStringList getValuesByClick        (QPoint pos) const;
    QString     getTitle                (QPoint pos) const;
    QString     getTenant               (QString title, int row) const;

    void populateHorizontal(SqlResultType sqlResult, QStringList selectedFields);
    void populateVertical  (SqlResultType sqlResult, QStringList selectedFields);

    void customContextMenuRequested(const QPoint& pos);
    void header_customContextMenuRequested(const QPoint& pos);
};

#endif // TABLEWIDGET_H
