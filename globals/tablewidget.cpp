#include "tablewidget.h"

#include <QHeaderView>
#include <QJsonArray>
#include <QClipboard>
#include <QDebug>

TableWidget::TableWidget(QWidget* parent)
    : QTableWidget(parent)
{
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    horizontalHeader()->setMouseTracking(true);
    horizontalHeader()->installEventFilter(this);

    connect(this              , &QWidget::customContextMenuRequested, this, &TableWidget::customContextMenuRequested);
    connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &TableWidget::header_customContextMenuRequested);
}

TableWidget::~TableWidget()
{
    resetWidget();
}

void TableWidget::resetWidget()
{
    clear();
    setRowCount(0);
    setColumnCount(0);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void TableWidget::filterResultFields(QStringList selectedFields)
{
    if(rowCount() <= 0)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto visibleItems = 0;
    if (m_currentViewMode == ViewMode::VM_VERTICAL)
    {
        for(int i = 0; i < rowCount(); ++i)
        {
            if(selectedFields.contains(item(i, 0)->text()) == false)
            {
                hideRow(i);
            }
            else
            {
                showRow(i);
                ++visibleItems;
            }
        }
    }
    else
    {
        for(int col = 0; col < columnCount(); ++col)
        {
            if(selectedFields.contains(horizontalHeaderItem(col)->text()) == false)
            {
                hideColumn(col);
            }
            else
            {
                showColumn(col);
                ++visibleItems;
            }
        }

    }
    emit visibleItemsCount(visibleItems);

    QApplication::restoreOverrideCursor();

}

void TableWidget::populate(SqlResultType sqlResult, QStringList selectedFields)
{
    resetWidget();

    if (sqlResult.count() <= 0)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
        populateHorizontal(sqlResult, selectedFields);
    else
        populateVertical(sqlResult, selectedFields);

    emit visibleItemsCount(rowCount());

    QApplication::restoreOverrideCursor();
}

QStringList TableWidget::getValuesByHeaderClick(QPoint pos) const
{
    QStringList values;
    auto column = horizontalHeader()->logicalIndexAt(pos);
    for(int row = 0; row < rowCount(); ++row)
        values.append(item(row, column)->text().trimmed());
    return values;
}

QStringList TableWidget::getValuesByClick(QPoint pos) const
{
    QStringList values;
    if (m_currentViewMode == VM_HORIZONTAL)
    {
        auto item = itemAt(pos);
        if (item != nullptr)
        {
            auto col = column(item);
            auto itemSelected = selectedItems();
            for(auto& currentItem : itemSelected)
            {   // only multiple values of the same column are allowed (in horizontal mode)
                if (column(currentItem) == col)
                    values.append(currentItem->text().trimmed());
            }
        }
    }
    return values;
}

QString TableWidget::getTitle(QPoint pos) const
{
    auto i = itemAt(pos);
    auto r = row(i);

    auto title = m_currentViewMode == ViewMode::VM_VERTICAL
            ? item(r, 0)->text()
            : horizontalHeaderItem(column(i))->text();
    return title;
}

void TableWidget::populateHorizontal(SqlResultType sqlResult, QStringList selectedFields)
{
    auto colTotal = sqlResult.at(0).count();
    auto record = sqlResult.at(0);
    setColumnCount(colTotal);

    for(auto& record : sqlResult)
    {
        insertRow(rowCount());

        int col = 0;
        for(auto i = record.begin(); i != record.end(); ++i)
        {
            auto itemValue = new QTableWidgetItem(i.value());
            itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
            setItem(rowCount() - 1, col++, itemValue);
        }
    }

    QStringList header;
    int col = 0;
    for(auto i = record.begin(); i != record.end(); ++i, ++col)
    {
        header.append(i.key());
        if (selectedFields.contains(i.key()) == false)
            hideColumn(col);
    }
    setHorizontalHeaderLabels(header);
}

void TableWidget::populateVertical(SqlResultType sqlResult, QStringList selectedFields)
{
    setColumnCount(2);
    setHorizontalHeaderLabels({tr("Campo"), tr("Valore")});

    int index = 1;
    for(auto& record : sqlResult)
    { // TODO: move constants somewhere else
        auto alternateColor = index % 2 == 0 ?
                    QColor::fromRgb(160, 160, 160) :
                    QColor::fromRgb(210, 210, 210);
        ++index;

        for(auto i = record.begin(); i != record.end(); ++i)
        {
            insertRow(rowCount());

            auto itemKey = new QTableWidgetItem(i.key());
            itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
            itemKey->setBackgroundColor(alternateColor);
            setItem(rowCount() - 1, 0, itemKey);

            auto itemValue = new QTableWidgetItem(i.value());
            itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
            itemValue->setBackgroundColor(alternateColor);
            setItem(rowCount() - 1, 1, itemValue);

            if (selectedFields.contains(i.key()) == false)
                hideRow(rowCount() - 1);
        }
    }
}

void TableWidget::customContextMenuRequested(const QPoint& pos)
{
    QTableWidgetItem* item = itemAt(pos);
    if (item == nullptr)
        return;

    auto values = getValuesByClick(pos);
    auto rowVal = row(item);
    auto title  = getTitle(pos);
    auto tenant = getTenant(title, rowVal);
    auto value  = values.isEmpty()
            ? item->text()
            : values.join(Globals::valuesSeparator());

    auto query = m_tenantQueryMap.contains(tenant) ? m_tenantQueryMap[tenant] : QString();
    auto tableNameAcronym = query.length() > 0
            ? Globals::getTableNameAndAcronym(title, tenant, query)
            : QPair<QString, QString>{};

    emit contextMenuRequest(
                {
                    {"stylesheet"  , styleSheet()},
                    {"tenant"      , tenant},
                    {"title"       , title},
                    {"value"       , value},
                    {"tableName"   , tableNameAcronym.first},
                    {"tableAcronym", tableNameAcronym.second}
                });
}

void TableWidget::header_customContextMenuRequested(const QPoint& pos)
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

QString TableWidget::getTenant(QString title, int row) const
{
    title = title.toLower();

    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
    {
        for(int col = 0; col < columnCount(); ++col)
        {
            if (horizontalHeaderItem(col)->text().toLower() == "tenant")
            {
                auto tenant = item(row, col)->text();
                return tenant;
            }
        }
    }
    else
    {
        QStringList list {"tenant", title};
        list.sort(); // we need to know if the tenant row is before or after the selected one
        if (list[0] == "tenant") // tenant is before
        {
            int rowCopy = row;
            while(rowCopy >= 0)
            {
                if (item(rowCopy, 0)->text().toLower() == "tenant")
                {
                    auto tenant = item(rowCopy, 1)->text();
                    return tenant;
                }
                --rowCopy;
            }
        }
        else // tenant is after
        {
            int rowCopy = row;
            while(rowCopy < rowCount())
            {
                if (item(rowCopy, 0)->text().toLower() == "tenant")
                {
                    auto tenant = item(rowCopy, 1)->text();
                    return tenant;
                }
                ++rowCopy;
            }
        }
    }
    return {};
}

bool TableWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == horizontalHeader() && m_currentViewMode == VM_HORIZONTAL)
    {
        if(event->type() == QEvent::Enter)
        {
            setCursor(Qt::PointingHandCursor);
            QApplication::processEvents();
        }
        else if(event->type() == QEvent::Leave)
        {
            setCursor(Qt::ArrowCursor);
            QApplication::processEvents();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TableWidget::keyPressEvent(QKeyEvent* event)
{
    const auto modifiers = event->modifiers();
    if(modifiers & Qt::ControlModifier && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::AltModifier))
    {
        const auto keyPressed = event->key();
        if(keyPressed == Qt::Key_C)
        {
            QString allItems;
            int currRow = -1;
            for(auto& item : selectedItems())
            {
                if(item->row() != currRow)
                {
                    if(currRow >= 0)
                        allItems.append("\n");
                    currRow = item->row();
                    allItems.append(item->text());
                }
                else
                {
                    allItems.append(QString("\t%1").arg(item->text()));
                }
            }
            auto clipboard = QApplication::clipboard();
            clipboard->setText(allItems);
        }
    }
    else
    {
        QAbstractItemView::keyPressEvent(event);
    }
}

