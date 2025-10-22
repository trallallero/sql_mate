#include "jsonmodel.h"
#include <QDebug>
#include <QFile>
#include <QFont>

bool contains(const QStringList& list, const QString &value)
{
    for (auto& val : list)
        if (value.contains(val, Qt::CaseInsensitive))
            return true;

    return false;
}

JsonTreeItem::JsonTreeItem(JsonTreeItem* parent)
{
    m_parent = parent;
}

JsonTreeItem::~JsonTreeItem()
{
    qDeleteAll(m_childs);
}

void JsonTreeItem::appendChild(JsonTreeItem* item)
{
    m_childs.append(item);
}

JsonTreeItem* JsonTreeItem::child(int row) const
{
    return m_childs.value(row);
}

JsonTreeItem* JsonTreeItem::parent() const
{
    return m_parent;
}

int JsonTreeItem::childCount() const
{
    return m_childs.count();
}

int JsonTreeItem::row() const
{
    if (m_parent)
        return m_parent->m_childs.indexOf(const_cast<JsonTreeItem*>(this));
    return 0;
}

void JsonTreeItem::setKey(const QString &key)
{
    m_key = key;
}

void JsonTreeItem::setValue(const QVariant &value)
{
    m_value = value;
}

void JsonTreeItem::setType(const QJsonValue::Type &type)
{
    m_type = type;
}

QString JsonTreeItem::key() const
{
    return m_key;
}

QVariant JsonTreeItem::value() const
{
    return m_value;
}

QJsonValue::Type JsonTreeItem::type() const
{
    return m_type;
}

JsonTreeItem* JsonTreeItem::load(const QJsonValue& value, const QStringList &exceptions, JsonTreeItem* parent)
{
    JsonTreeItem* rootItem = new JsonTreeItem(parent);
    rootItem->setKey("root");

    if (value.isObject())
    {
        //Get all QJsonValue childs
        auto keys = value.toObject().keys(); // To prevent clazy-range warning
        for (auto& key : keys)
        {
            if (contains(exceptions, key))
                continue;

            QJsonValue v = value.toObject().value(key);
            JsonTreeItem* child = load(v, exceptions, rootItem);
            child->setKey(key);
            child->setType(v.type());
            rootItem->appendChild(child);
        }
    }
    else if (value.isArray())
    {
        //Get all QJsonValue childs
        auto index = 0;
        const auto array = value.toArray(); // To prevent clazy-range warning
        for (const auto& v : array)
        {
            auto child = load(v, exceptions, rootItem);
            child->setKey(QString::number(index));
            child->setType(v.type());
            rootItem->appendChild(child);
            ++index;
        }
    }
    else
    {
        rootItem->setValue(value.toVariant());
        rootItem->setType (value.type());
    }
    return rootItem;
}

//=========================================================================

inline uchar hexdig(uint u)
{
    return (u < 0xa ? '0' + u : 'a' + u - 0xa);
}

QByteArray escapedString(const QString &s)
{
    QByteArray ba(s.length(), Qt::Uninitialized);
    uchar* cursor = reinterpret_cast<uchar*>(const_cast<char*>(ba.constData()));
    const uchar* ba_end = cursor + ba.length();
    const ushort* src = reinterpret_cast<const ushort*>(s.constBegin());
    const ushort* const end = reinterpret_cast<const ushort*>(s.constEnd());
    while (src != end)
    {
        if (cursor >= ba_end - 6)
        {
            // ensure we have enough space
            int pos = cursor - reinterpret_cast<const uchar*>(ba.constData());
            ba.resize(ba.size() * 2);
            cursor = reinterpret_cast<uchar*>(ba.data()) + pos;
            ba_end = reinterpret_cast<const uchar*>(ba.constData()) + ba.length();
        }
        uint u = *src++;
        if (u < 0x80)
        {
            if (u < 0x20 || u == 0x22 || u == 0x5c)
            {
                *cursor++ = '\\';
                switch (u)
                {
                    case 0x22:
                        *cursor++ = '"';
                        break;
                    case 0x5c:
                        *cursor++ = '\\';
                        break;
                    case 0x8:
                        *cursor++ = 'b';
                        break;
                    case 0xc:
                        *cursor++ = 'f';
                        break;
                    case 0xa:
                        *cursor++ = 'n';
                        break;
                    case 0xd:
                        *cursor++ = 'r';
                        break;
                    case 0x9:
                        *cursor++ = 't';
                        break;
                    default:
                        *cursor++ = 'u';
                        *cursor++ = '0';
                        *cursor++ = '0';
                        *cursor++ = hexdig(u >> 4);
                        *cursor++ = hexdig(u & 0xf);
                }
            }
            else
            {
                *cursor++ = (uchar)u;
            }
        }
        else if (Utf8Functions::toUtf8<QUtf8BaseTraits>(u, cursor, src, end) < 0)
        {
            // failed to get valid utf8 use JSON escape sequence
            *cursor++ = '\\';
            *cursor++ = 'u';
            *cursor++ = hexdig(u >> 12 & 0x0f);
            *cursor++ = hexdig(u >> 8 & 0x0f);
            *cursor++ = hexdig(u >> 4 & 0x0f);
            *cursor++ = hexdig(u & 0x0f);
        }
    }
    ba.resize(cursor - reinterpret_cast<const uchar*>(ba.constData()));
    return ba;
}

JsonModel::JsonModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootItem{new JsonTreeItem}
{
    m_headers.append(tr("Chiave"));
    m_headers.append(tr("Valore"));
}

JsonModel::~JsonModel()
{
    delete m_rootItem;
}

bool JsonModel::fromJson(const QByteArray &json)
{
    auto const& jdoc = QJsonDocument::fromJson(json);

    if (!jdoc.isNull())
    {
        beginResetModel();
        delete m_rootItem;
        if (jdoc.isArray())
        {
            m_rootItem = JsonTreeItem::load(QJsonValue(jdoc.array()), m_exceptions);
            m_rootItem->setType(QJsonValue::Array);

        }
        else
        {
            m_rootItem = JsonTreeItem::load(QJsonValue(jdoc.object()), m_exceptions);
            m_rootItem->setType(QJsonValue::Object);
        }
        endResetModel();
        return true;
    }
    return false;
}

QVariant JsonModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    JsonTreeItem* item = static_cast<JsonTreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
            return QString("%1").arg(item->key());

        if (index.column() == 1)
            return item->value();
    }
    return {};
}

QVariant JsonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        return m_headers.value(section);
    return {};
}

QModelIndex JsonModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    JsonTreeItem* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<JsonTreeItem*>(parent.internalPointer());

    JsonTreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex JsonModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    JsonTreeItem* childItem = static_cast<JsonTreeItem*>(index.internalPointer());
    JsonTreeItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int JsonModel::rowCount(const QModelIndex& parent) const
{
    JsonTreeItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<JsonTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int JsonModel::columnCount(const QModelIndex&) const
{
    return 2;
}

Qt::ItemFlags JsonModel::flags(const QModelIndex& index) const
{
    int col   = index.column();
    auto item = static_cast<JsonTreeItem*>(index.internalPointer());

    auto isArray  = QJsonValue::Array  == item->type();
    auto isObject = QJsonValue::Object == item->type();

    if ((col == 1) && !(isArray || isObject))
        return QAbstractItemModel::flags(index);
    return QAbstractItemModel::flags(index);
}

QByteArray JsonModel::json(bool compact)
{
    auto jsonValue = genJson(m_rootItem);
    QByteArray json;
    if (jsonValue.isNull())
        return json;

    if (jsonValue.isArray())
        arrayToJson(jsonValue.toArray(), json, 0, compact);
    else
        objectToJson(jsonValue.toObject(), json, 0, compact);

    return json;
}

void JsonModel::objectToJson(QJsonObject jsonObject, QByteArray& json, int indent, bool compact)
{
    json += compact ? "{" : "{\n";
    objectContentToJson(jsonObject, json, indent + (compact ? 0 : 1), compact);
    json += QByteArray(4 * indent, ' ');
    json += compact ? "}" : "}\n";
}
void JsonModel::arrayToJson(QJsonArray jsonArray, QByteArray& json, int indent, bool compact)
{
    json += compact ? "[" : "[\n";
    arrayContentToJson(jsonArray, json, indent + (compact ? 0 : 1), compact);
    json += QByteArray(4 * indent, ' ');
    json += compact ? "]" : "]\n";
}

void JsonModel::arrayContentToJson(QJsonArray jsonArray, QByteArray& json, int indent, bool compact)
{
    if (jsonArray.size() <= 0)
        return;

    QByteArray indentString(4 * indent, ' ');
    int i = 0;
    while (1)
    {
        json += indentString;
        valueToJson(jsonArray.at(i), json, indent, compact);
        if (++i == jsonArray.size())
        {
            if (!compact)
                json += '\n';
            break;
        }
        json += compact ? "," : ",\n";
    }
}

void JsonModel::objectContentToJson(QJsonObject jsonObject, QByteArray& json, int indent, bool compact)
{
    if (jsonObject.size() <= 0)
        return;

    QByteArray indentString(4 * indent, ' ');
    int i = 0;
    while (1)
    {
        QString key = jsonObject.keys().at(i);
        json += indentString;
        json += '"';
        json += escapedString(key);
        json += compact ? "\":" : "\": ";
        valueToJson(jsonObject.value(key), json, indent, compact);
        if (++i == jsonObject.size())
        {
            if (!compact)
                json += '\n';
            break;
        }
        json += compact ? "," : ",\n";
    }
}

void JsonModel::valueToJson(QJsonValue jsonValue, QByteArray& json, int indent, bool compact)
{
    QJsonValue::Type type = jsonValue.type();
    switch (type)
    {
        case QJsonValue::Bool:
            json += jsonValue.toBool() ? "true" : "false";
            break;
        case QJsonValue::Double:
        {
            const double d = jsonValue.toDouble();
            if (qIsFinite(d))
                json += QByteArray::number(d, 'f', QLocale::FloatingPointShortest);
            else
                json += "null";
            break;
        }
        case QJsonValue::String:
            json += '"';
            json += escapedString(jsonValue.toString());
            json += '"';
            break;
        case QJsonValue::Array:
            json += compact ? "[" : "[\n";
            arrayContentToJson(jsonValue.toArray(), json, indent + (compact ? 0 : 1), compact);
            json += QByteArray(4 * indent, ' ');
            json += ']';
            break;
        case QJsonValue::Object:
            json += compact ? "{" : "{\n";
            objectContentToJson(jsonValue.toObject(), json, indent + (compact ? 0 : 1), compact);
            json += QByteArray(4 * indent, ' ');
            json += '}';
            break;
        case QJsonValue::Null:
        default:
            json += "null";
    }
}

void JsonModel::addException(const QStringList &exceptions)
{
    m_exceptions = exceptions;
}

QJsonValue JsonModel::genJson(JsonTreeItem* item) const
{
    auto type   = item->type();
    int  nchild = item->childCount();

    if (QJsonValue::Object == type)
    {
        QJsonObject jo;
        for (int i = 0; i < nchild; ++i)
        {
            auto ch = item->child(i);
            auto key = ch->key();
            jo.insert(key, genJson(ch));
        }
        return  jo;
    }
    else if (QJsonValue::Array == type)
    {
        QJsonArray arr;
        for (int i = 0; i < nchild; ++i)
        {
            auto ch = item->child(i);
            arr.append(genJson(ch));
        }
        return arr;
    }
    else
    {
        QJsonValue va;
        switch(item->value().type())
        {
            case QVariant::Bool:
            {
                va = item->value().toBool();
                break;
            }
            default:
                va = item->value().toString();
                break;
        }
        return va;
    }
}
