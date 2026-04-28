#include "pluginshandler.h"
#include "globals.h"

#include <QDir>
#include <QApplication>

PluginsHandler::PluginsHandler(QWidget* parent)
    : m_parent(parent)
    , m_searchMethodRequest([this]() {
        emit PluginsHandler::searchMethodRequest();
      })
    , m_messageMethodRequest([this](QString message) {
        emit PluginsHandler::messageMethodRequest(message);
      })
    , m_addConditionMethodRequest([this](QJsonObject jo) {
        emit PluginsHandler::addConditionMethodRequest(jo);
    })
    , m_setSqlConnectionMethodRequest([this](QString connectionName, bool showMessage) {
        emit PluginsHandler::setSqlConnectionMethodRequest(connectionName, showMessage);
    })
    , m_setExportResultMethodRequest([this](QWidget* parent, QTableWidget* tw, ViewMode vm) {
        emit PluginsHandler::setExportResultMethodRequest(parent, tw, vm);
    })
    , m_setContextMenuMethodRequest([this](QWidget*, QJsonObject data) {
        setData(data);
        m_shadowMenuWidget->popup();
    })
    , m_setSqlQueryMethodRequest([this](QString tenant, QString query, QObject* sender) {
        emit PluginsHandler::setSqlQueryMethodRequest(tenant, query, sender);
    })
    , m_setGetQueryMethodRequest([this](QString tenant, QString& query) {
        emit PluginsHandler::setGetQueryMethodRequest(tenant, query);
    })
{
    m_shadowMenuWidget = new ContextMenuWidget(&m_contextMenu);
    m_contextMenu.setObjectName("contextMenu_1");
}

void PluginsHandler::loadPlugins()
{
    QDir mainPluginsDir;
    mainPluginsDir.cd(Globals::pluginsDirectory());

    const QStringList dirEntries = mainPluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto& dirName : dirEntries)
    {
        QDir pluginsDir;
        pluginsDir.cd(Globals::pluginsDirectory() + "/" + dirName);

        const QStringList entries = pluginsDir.entryList(QDir::Files);
        for (auto& fileName : entries)
        {
            if (fileName.endsWith(".dll") == false)
                continue;

            m_pluginLoader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName), this);
            QObject* plugin = m_pluginLoader->instance();
            //auto error = pluginLoader.errorString();
            if (plugin)
            {
                auto pluginInterface = qobject_cast<PluginInterface *>(plugin);
                if (pluginInterface)
                {
                    pluginInterface->setSearchMethod         (m_searchMethodRequest);
                    pluginInterface->setMessageMethod        (m_messageMethodRequest);
                    pluginInterface->setAddConditionMethod   (m_addConditionMethodRequest);
                    pluginInterface->setSqlConnectionMethod  (m_setSqlConnectionMethodRequest);
                    pluginInterface->setExportResultMethod   (m_setExportResultMethodRequest);
                    pluginInterface->setContextMenuMethod    (m_setContextMenuMethodRequest);
                    pluginInterface->setSqlRequestMethod     (m_setSqlQueryMethodRequest);
                    pluginInterface->setGetQueryRequestMethod(m_setGetQueryMethodRequest);


                    auto iconFilename = pluginInterface->getIconFilename();
                    QString iconDir = pluginsDir.canonicalPath() + "/" + iconFilename;
                    QAction* action { nullptr };
                    if (pluginInterface->getType() == PT_TableWidget)
                    {
                        action = m_contextMenu.addAction(QIcon(iconDir), pluginInterface->getName(), this, &PluginsHandler::executePluginAction);
                        m_contextMenu.addSeparator();
                    }
                    else if (pluginInterface->getType() == PT_MainMenu)
                    {
                        action = m_mainMenu.addAction(QIcon(iconDir), pluginInterface->getName(), this, &PluginsHandler::executePluginAction);
                        m_contextMenu.addSeparator();
                    }

                    if(action != nullptr)
                        m_actionPluginMap[action] = pluginInterface;
                }
            }
        }
    }
}

void PluginsHandler::unloadPlugins()
{
    for(auto item : m_actionPluginMap.toStdMap())
        delete item.second;
}

void PluginsHandler::setCurrentSqlDB(QString connectionName, QSqlDatabase* sqlDB)
{
    for (auto iter : m_actionPluginMap.toStdMap())
    {
        auto& pluginInterface = m_actionPluginMap[iter.first];
        pluginInterface->setCurrentSqlDB(connectionName, sqlDB);
    }
}

void PluginsHandler::setSqlDBs(QStringList connectionNames)
{
    for (auto iter : m_actionPluginMap.toStdMap())
    {
        auto& pluginInterface = m_actionPluginMap[iter.first];
        pluginInterface->setSqlDBs(connectionNames);
    }
}

void PluginsHandler::setData(QJsonObject jo)
{
    for (auto iter : m_actionPluginMap.toStdMap())
    {
        auto& pluginInterface = m_actionPluginMap[iter.first];
        pluginInterface->setData(jo);
        iter.first->setEnabled(pluginInterface->isEnabled());
    }
}

void PluginsHandler::languageChanged()
{
    for (auto iter : m_actionPluginMap.toStdMap())
    {
        auto& pluginInterface = m_actionPluginMap[iter.first];
        pluginInterface->setLanguageChanged(true);
    }
}

void PluginsHandler::executePluginAction()
{
    auto* action = static_cast<QAction*>(sender());
    if (action && m_actionPluginMap.contains(action))
    {
        auto* pluginInterface = m_actionPluginMap[action];
        if (pluginInterface)
        {
            pluginInterface->execute();
        }
    }
}
