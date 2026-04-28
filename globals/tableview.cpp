#include "tableview.h"

#include <QJsonArray>
#include <QStandardItem>

TableView::TableView(QWidget* parent) : QTableView(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    horizontalHeader()->setMouseTracking(true);
    horizontalHeader()->installEventFilter(this);

    connect(this              , &QWidget::customContextMenuRequested, this, &TableView::customContextMenuRequested);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &TableView::header_customContextMenuRequested);

    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

}

TableView::~TableView()
{
}

void TableView::customContextMenuRequested(const QPoint& pos)
{
    auto index = indexAt(pos);

    auto values = getValuesByClick(pos);
    auto rowVal = indexAt(pos).row();
    auto title  = getTitle(pos);
    auto tenant = getTenant(title, rowVal);
    auto value  = values.isEmpty()
            ? model()->data(index, Qt::DisplayRole).toString()
            : values.join(Globals::valuesSeparator());

    auto query = m_tenantQueryMap.contains(tenant) ? m_tenantQueryMap[tenant] : QString();
    auto tableNameAcronym = query.length() > 0
            ? Globals::getTableNameAndAcronym(title, tenant, query)
            : QPair<QString, QString>{};

    emit contextMenuRequest(
                {
                    {"stylesheet"  , styleSheet()            },
                    {"tenant"      , tenant                  },
                    {"title"       , title                   },
                    {"value"       , value                   },
                    {"tableName"   , tableNameAcronym.first  },
                    {"tableAcronym", tableNameAcronym.second }
                }
   );
}

void TableView::header_customContextMenuRequested(const QPoint& pos)
{
    QStringList values = getValuesByHeaderClick(pos);
    auto value = values.join(Globals::valuesSeparator());

    emit contextMenuRequest(
                {
                    {"stylesheet", styleSheet()},
                    {"value"     , value}
                }
    );
}

QStringList TableView::getValuesByHeaderClick(QPoint pos) const
{
    QStringList values;
    auto column = horizontalHeader()->logicalIndexAt(pos);
    for(int row = 0; row < model()->rowCount(); ++row)
    {
        auto index = model()->index(row, column);
        auto value = model()->data(index, Qt::DisplayRole);

        values.append(value.toString().trimmed());
    }
    return values;
}

QStringList TableView::getValuesByClick(QPoint pos) const
{
    QStringList values;
    auto index = indexAt(pos);
    auto selected = selectedIndexes();
    for(auto& selIndex : selected)
    {
        if (selIndex.column() == index.column())
            values.append(model()->data(selIndex, Qt::DisplayRole).toString().trimmed());
    }
    return values;
}

QString TableView::getTitle(QPoint pos) const
{
    auto index = indexAt(pos);
    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
        return model()->headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
    return model()->data(model()->index(index.row(), 0)).toString();
}

QString TableView::getTenant(QString title, int row) const
{
    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
    {
        for(int col = 0; col < model()->columnCount(); ++col)
        {
            if (model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString().toLower() == "tenant")
            {
                auto index = model()->index(row, col);
                auto tenant = model()->data(index, Qt::DisplayRole).toString().trimmed();
                return tenant;
            }
        }
    }
    else
    {
        QStringList list {"tenant", title.toLower()};
        list.sort(); // we need to know if the tenant row is before or after the selected one
        if (list[0] == "tenant") // tenant is before
        {
            while (--row >= 0)
            {
                if (model()->data(model()->index(row, 0)).toString().toLower() == "tenant")
                    return model()->data(model()->index(row, 1)).toString();
            }
        }
        else
        {
            while (++row < model()->rowCount())
            {
                if (model()->data(model()->index(row, 0)).toString().toLower() == "tenant")
                    return model()->data(model()->index(row, 1)).toString();
            }
        }
    }
    return {};
}
