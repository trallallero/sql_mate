#ifndef PLUGINSHANDLER_H
#define PLUGINSHANDLER_H

#include "plugininterface.h"
#include "contextmenuwidget.h"

#include <QObject>
#include <QAction>
#include <QMap>
#include <QMenu>
#include <QPluginLoader>

class Conditions;

class PluginsHandler : public QObject
{
    Q_OBJECT

public:
    PluginsHandler(QWidget* parent);

    void loadPlugins();
    void unloadPlugins();

    //QMenu* getContextMenu() { return &m_contextMenu; }
    ContextMenuWidget* getContextMenuWidget() { return m_shadowMenuWidget; }
    QMenu*             getMainMenu         () { return &m_mainMenu; }

    void setCurrentSqlDB(QString connectionName, QSqlDatabase* sqlDB);
    void setSqlDBs      (QStringList connectionNames);

    void setData(QJsonObject jo);

    void languageChanged();

signals:
    void searchMethodRequest          ();
    void messageMethodRequest         (QString);
    void addConditionMethodRequest    (QJsonObject);
    void setSqlConnectionMethodRequest(QString, bool);
    void setExportResultMethodRequest (QWidget*, QTableWidget*, ViewMode);
    void setSqlQueryMethodRequest     (QString, QString, QObject*);
    void setGetQueryMethodRequest     (QString, QString&);

private slots:
    void executePluginAction();

private:
    QMap<QAction*, PluginInterface*> m_actionPluginMap;
    QMenu                            m_contextMenu;
    ContextMenuWidget*               m_shadowMenuWidget { nullptr };
    QMenu                            m_mainMenu;
    QWidget*                         m_parent;
    QPluginLoader*                   m_pluginLoader { nullptr };

    std::function<void()                                 > m_searchMethodRequest;
    std::function<void(QString)                          > m_messageMethodRequest;
    std::function<void(QJsonObject)                      > m_addConditionMethodRequest;
    std::function<void(QString, bool)                    > m_setSqlConnectionMethodRequest;
    std::function<void(QWidget*, QTableWidget*, ViewMode)> m_setExportResultMethodRequest;
    std::function<void(QWidget*, QJsonObject)            > m_setContextMenuMethodRequest;
    std::function<void(QString, QString, QObject*)       > m_setSqlQueryMethodRequest;
    std::function<void(QString, QString&)                > m_setGetQueryMethodRequest;
};
#endif // PLUGINSHANDLER_H
