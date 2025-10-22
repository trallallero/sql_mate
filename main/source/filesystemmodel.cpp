#include "filesystemmodel.h"

#include <QFont>
#include <QDebug>

FileSystemModel::FileSystemModel(QObject* parent, QString currentProfile, QString currentSchema)
    : QFileSystemModel(parent)
    , m_currentProfile(currentProfile)
    , m_currentSchema(currentSchema)
{
}

QVariant FileSystemModel::data(const QModelIndex& index, int role) const
{
//    if (role == Qt::DisplayRole && index.isValid())
//        qDebug() << index.data().toString();

    if (role == Qt::FontRole)
    {
        qDebug() << index.data().toString();
        if (index.data().toString() == m_currentProfile)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        if(index.parent().data().toString() == m_currentProfile && index.data().toString() == m_currentSchema)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
    }
    return QFileSystemModel::data(index, role);
}
