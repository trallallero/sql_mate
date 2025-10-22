#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit FileSystemModel(QObject* parent, QString currentProfile, QString currentSchema);

    virtual QVariant data    (const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setRowsInserted(int rowCount) { m_rowCount = rowCount; }

private:
    QString m_currentProfile;
    QString m_currentSchema;
    int     m_rowCount {0};
};

#endif // FILESYSTEMMODEL_H
