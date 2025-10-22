#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <functional>

#include <QApplication>
#include <QWidget>
#include <QtPlugin>
#include <QString>
#include <QTableWidget>

#include "globals.h"

class QSqlDatabase;

typedef enum {
    PT_MainMenu,    // plugin for the app main menu
    PT_TableWidget  // plugin for the result table widget
} PluginType;

class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    virtual QString    getName            () const = 0;
    virtual QString    getIconFilename    () const = 0;
    virtual void       execute            ()       = 0;
    virtual bool       isEnabled          ()       = 0;
    virtual PluginType getType            () const = 0;

    virtual QDialog* createPluginGui(QDialog* parent) { return Globals::createShadowedGui(parent, true); }

    void setCurrentSqlDB(QString connectionName, QSqlDatabase* db)
    {
        m_currentConnectionName = connectionName;
        m_currentDb = db;
    }

    void setSqlDBs(QStringList connectionNames) { m_connectionNames = connectionNames; }

    QSqlDatabase* sqlDB() const { return m_currentDb; }

    QStringList connectionsNames() const { return m_connectionNames; }
    QString     connectionsName () const { return m_currentConnectionName; }

    void        setData(QJsonObject jo) { m_data = jo; }
    QJsonObject getData() const         { return m_data; }

    bool languageChanged() { return m_languageChanged; }
    void setLanguageChanged(bool changed) { m_languageChanged = changed; }

    const QWidget* getMainWidget() const
    {
        auto widgets = QApplication::topLevelWidgets();
        for(auto& w : widgets)
            if (w->objectName() == "MainWindow")
                return w;
        return nullptr;
    }

    void setSearchMethod         (std::function<void()                                 > method) { m_searchMethod          = method; }
    void setMessageMethod        (std::function<void(QString)                          > method) { m_messageMethod         = method; }
    void setAddConditionMethod   (std::function<void(QJsonObject)                      > method) { m_addConditionMethod    = method; }
    void setSqlConnectionMethod  (std::function<void(QString, bool)                    > method) { m_sqlConnectionMethod   = method; }
    void setExportResultMethod   (std::function<void(QWidget*, QTableWidget*, ViewMode)> method) { m_exportResultMethod    = method; }
    void setContextMenuMethod    (std::function<void(QWidget*, QJsonObject)            > method) { m_contextMenuMethod     = method; }
    void setSqlRequestMethod     (std::function<void(QString, QString, QObject*)       > method) { m_sqlQueryRequestMethod = method; }
    void setGetQueryRequestMethod(std::function<void(QString, QString&)                > method) { m_getQueryRequestMethod = method; }

    std::function<void()                                 > m_searchMethod          { nullptr };
    std::function<void(QString)                          > m_messageMethod         { nullptr };
    std::function<void(QJsonObject)                      > m_addConditionMethod    { nullptr };
    std::function<void(QString, bool)                    > m_sqlConnectionMethod   { nullptr };
    std::function<void(QWidget*, QTableWidget*, ViewMode)> m_exportResultMethod    { nullptr };
    std::function<void(QWidget*, QJsonObject)            > m_contextMenuMethod     { nullptr };
    std::function<void(QString, QString, QObject*)       > m_sqlQueryRequestMethod { nullptr };
    std::function<void(QString, QString&)                > m_getQueryRequestMethod { nullptr };

private:
    QJsonObject   m_data;
    QString       m_currentConnectionName;
    QSqlDatabase* m_currentDb;
    QStringList   m_connectionNames;
    bool          m_languageChanged { false };
};

#define PluginInterface_iid "org.marco.servadei.PluginInterface"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
