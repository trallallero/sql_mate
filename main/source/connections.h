#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <QDialog>
#include <QListWidgetItem>
#include <QJsonArray>

namespace Ui {
class Connections;
}

struct ConnectionValues
{
    QString name;
    QString host;
    QString db;
    QString user;
    QString pwd;
    bool    isMultiDb;
};

class Connections : public QDialog
{
    Q_OBJECT

public:
    explicit Connections(QWidget* parent = nullptr);
    ~Connections();

    static QStringList getConnectionNames();
    static ConnectionValues getConnectionValues(QString name);

signals:
    void connectionAdded   (QString connectionName) const;
    void connectionModified(QString oldConnectionName, QString newConnectionName) const;
    void showMessage(QString message) const;

private slots:
    void pushButtonClose_clicked();
    void pushButtonNew_clicked();
    void pushButtonSave_clicked();
    void pushButtonTest_clicked();
    void pushButtonEdit_clicked();
    void pushButtonDelete_clicked();
    void pushButtonCancel_clicked();

    void lineEditName_textChanged(const QString& arg1);
    void listWidgetConnections_itemClicked(QListWidgetItem* item);

    void toolButtonCopyError_clicked();

private:
    Ui::Connections* ui;
    QDialog*         m_dialog          { nullptr };
    QListWidgetItem* m_currentItem     { nullptr };
    QString          m_currentItemText {};
    bool             m_isEditing       { false   };

    static QJsonArray m_connections;

    static void loadConnections();

    void connectObjects();
    void showConnections();
    void removeCurrentItem();

    void clearValues();
    void enableValues(bool enable);
};

#endif // CONNECTIONS_H
