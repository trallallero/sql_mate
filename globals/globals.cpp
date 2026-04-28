#include "globals.h"

#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <QFile>
#include <QDebug>

QString  Globals::m_templateDirectory;
QString  Globals::m_defaultDirectory;
QString  Globals::m_profilesDirectory;
QString  Globals::m_configDirectory;
QString  Globals::m_pluginsDirectory;
QString  Globals::m_filenameQuery;
QString  Globals::m_filenameConditionFields;
QString  Globals::m_filenameConfig;
QString  Globals::m_filenameSchemas;
QString  Globals::m_filenameTenantsQuery;
QString  Globals::m_filenameSelectedFields;
QString  Globals::m_filenameStyleSheet;
QString  Globals::m_filenameConnections;
QString  Globals::m_filenameProfileReadMe;
QString  Globals::m_applicationName;
QString  Globals::m_newConnectionString;
QString  Globals::m_valuesSeparator;
QString  Globals::m_profile;
QString  Globals::m_schema;
int      Globals::m_selectedFieldMilliseconds;
int      Globals::m_sqlConnectTimeout;
int      Globals::m_numberOfGroups;
int      Globals::m_maxNumberOfConditionsPerGroup;

std::function<void(QWidget*, QString)> Globals::m_onConfigSavedMethod = nullptr;

void Globals::initialize()
{
    auto jsonObj = getJsonObject("config/globals.json");

    m_templateDirectory             = jsonObj["templateDirectory"            ].toString();
    m_defaultDirectory              = jsonObj["defaultDirectory"             ].toString();
    m_profilesDirectory             = jsonObj["profilesDirectory"            ].toString();
    m_configDirectory               = jsonObj["configDirectory"              ].toString();
    m_pluginsDirectory              = jsonObj["pluginsDirectory"             ].toString();
    m_filenameQuery                 = jsonObj["filenameQuery"                ].toString();
    m_filenameConditionFields       = jsonObj["filenameConditionFields"      ].toString();
    m_filenameConfig                = jsonObj["filenameConfig"               ].toString();
    m_filenameSchemas               = jsonObj["filenameSchemas"              ].toString();
    m_filenameTenantsQuery          = jsonObj["filenameTenantsQuery"         ].toString();
    m_filenameSelectedFields        = jsonObj["filenameSelectedFields"       ].toString();
    m_filenameStyleSheet            = jsonObj["filenameStyleSheet"           ].toString();
    m_filenameConnections           = jsonObj["filenameConnections"          ].toString();
    m_filenameProfileReadMe         = jsonObj["filenameProfileReadMe"        ].toString();
    m_applicationName               = jsonObj["applicationName"              ].toString();
    m_newConnectionString           = jsonObj["newConnectionString"          ].toString();
    m_valuesSeparator               = jsonObj["valuesSeparator"              ].toString();
    m_selectedFieldMilliseconds     = jsonObj["selectedFieldMilliseconds"    ].toInt();
    m_sqlConnectTimeout             = jsonObj["sqlConnectTimeout"            ].toInt();
    m_numberOfGroups                = jsonObj["numberOfGroups"               ].toInt();
    m_maxNumberOfConditionsPerGroup = jsonObj["maxNumberOfConditionsPerGroup"].toInt();
}

void Globals::setApplicationStyleSheet(QApplication* app)
{
    QFile file(Globals::filenameConfigStyleSheet());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    auto stylesheet = file.readAll();
    app->setStyleSheet(stylesheet);
}

void Globals::setProfileToDefault(const QString profile)
{
    auto configObj = Globals::globalConfigurationObject();
    if (configObj["defaultProfile"] != profile)
    {
        configObj["defaultProfile"] = profile;
        saveGlobalConfigurationObject(configObj, nullptr, {});
    }
}

QString Globals::getProfileSetToDefault()
{
    auto configObj = Globals::globalConfigurationObject();
    return configObj["defaultProfile"].toString();
}

void Globals::setSchemaToDefault(const QString schema)
{
    auto configObj = Globals::profileSchemasObject();
    if (configObj[Globals::defaultDirectory()] != schema)
    {
        configObj[Globals::defaultDirectory()] = schema;
        QFile fileConfig(Globals::filenameProfileSchemas());
        fileConfig.open(QIODevice::WriteOnly);
        fileConfig.write(QJsonDocument(configObj).toJson());
        fileConfig.close();
    }
}

QString Globals::getSchemaSetToDefault()
{
    auto configObj = Globals::profileSchemasObject();
    return configObj[Globals::defaultDirectory()].toString();
}

QString Globals::filenameProfileConfig()
{
    return m_schema.isEmpty()
            ? currentProfileDirectory() + m_filenameConfig
            : currentSchemaDirectory()  + m_filenameConfig;
}

QJsonObject Globals::profileConfigurationObject()
{
    return m_schema.isEmpty()
            ? getJsonObject(Globals::filenameProfileConfig())
            : getJsonObject(Globals::filenameSchemaConfig ());
}

QJsonObject Globals::globalConfigurationObject()
{
    return getJsonObject(Globals::filenameConfigConfig());
}

QJsonObject Globals::profileSchemasObject()
{
    return getJsonObject(Globals::filenameProfileSchemas());
}

void Globals::saveGlobalConfigurationObject(QJsonObject& configObj, QWidget* w, QString message)
{
    QFile fileConfig(Globals::filenameConfigConfig());
    fileConfig.open(QIODevice::WriteOnly);
    fileConfig.write(QJsonDocument(configObj).toJson());
    fileConfig.close();

    if(m_onConfigSavedMethod && w && message.isEmpty() == false)
        m_onConfigSavedMethod(w, message);
}

QMap<QString, QString> Globals::getAllProfileConditionsMap(QString profile)
{
    QMap<QString, QString> allConditions;
    QFile file_query(Globals::filenameProfileQuery(profile));
    file_query.open(QIODevice::ReadOnly | QIODevice::Text);
    auto query = QString(file_query.readAll());
    file_query.close();

    QRegularExpression re("^(.*) AS \"(.*)\".*$");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i = re.globalMatch(query);
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QString field = match.captured(1).trimmed();
        QString name  = match.captured(2).trimmed();
        allConditions[name] = field;
    }
    return allConditions;
}

bool Globals::copyPath(QString src, QString dst)
{
    QDir dirSrc(src);
    if (dirSrc.exists() == false)
        return false;

    QDir dirDst(dst);
    if (dirDst.exists() == false)
        QDir().mkpath(dst);

    for(auto& d : dirSrc.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        auto dstPath = dst + QDir::separator() + d;
        dirSrc.mkpath(dstPath);
        copyPath(src + QDir::separator() + d, dstPath);
    }

    for(auto& f : dirSrc.entryList(QDir::Files))
    {
        QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
    }
    return true;
}

QPair<QString, QString> Globals::getTableNameAndAcronym(QString title, QString tenant, QString query)
{
    auto lines = query.split("\n");
    QString table{}, tableAcronym{};
    for (auto& line  : lines)
    {
        if (tableAcronym.isEmpty())
        {
            if (line.contains("\"" + title + "\""))
            {
                QRegularExpression re("^.*[()\\s,](\\w+\\.[\"]*\\w+[\"]*).*$");
                QRegularExpressionMatch match = re.match(line);
                if (match.hasMatch())
                {
                    tableAcronym = match.captured(1).split(".").at(0);

                }
            }
        }
        else
        {
            QRegularExpression re(QString("^.*([\" ]%1[\"]?\\.*[^ ]*)[ ]+(%2)").arg(tenant, tableAcronym));
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch())
            {
                table = match.captured(1).trimmed()
                        .replace(tenant, "")
                        .replace("\"\".", "")
                        .replace(QRegularExpression("^\\."), "");;
                break;
            }
        }
    }
    return { table, tableAcronym };
}

void Globals::setWidgetShadow(QWidget* parent, QWidget* w, int shadow)
{
    if(shadow == 0)
    {
        w->setGraphicsEffect(nullptr);
    }
    else
    {
        auto effect = new QGraphicsDropShadowEffect(parent);
        effect->setBlurRadius(shadow);
        effect->setXOffset   (shadow);
        effect->setYOffset   (shadow);
        effect->setColor(QColor::fromRgb(60, 60, 60));
        w->setGraphicsEffect(effect);
    }
}

Dialog* Globals::createShadowedGui(QDialog* parent, bool isPlugin)
{
    parent->setWindowFlags(parent->windowFlags() | Qt::FramelessWindowHint);

    // Translucent background is needed in order for the shadow to work.
    parent->setAttribute(Qt::WA_TranslucentBackground, true);

    auto dialog = new Dialog();
    if(isPlugin)
    {
        dialog->setProperty("widgetType", "SQLMatePlugin");
    }

    auto layout = new QHBoxLayout();
    layout->addWidget(dialog);
    parent->setLayout(layout);

    setWidgetShadow(parent, dialog, 8);

    // dialog->setVisible(true) because next time parent will show, dialog will be visible
    parent->connect(dialog, &QDialog::finished, parent, [parent, dialog](int result){
        parent->done  (result);
        dialog->setVisible(true);
    });
    parent->connect(dialog, &QDialog::rejected, parent, [parent, dialog](){
        parent->reject();
        dialog->setVisible(true);
    });
    parent->connect(dialog, &QDialog::accepted, parent, [parent, dialog](){
        parent->accept();
        dialog->setVisible(true);
    });

    return dialog;
}

QColor Globals::getAlternateColor(int& index, bool increaseIndex)
{
    if (increaseIndex)
        ++index;
    return index % 2 == 0
        ? QColor::fromRgb(160, 160, 160)
        : QColor::fromRgb(210, 210, 210);
}

QJsonObject Globals::getJsonObject(QString file)
{
    QFile inputFileConfig(file);
    inputFileConfig.open(QIODevice::ReadOnly | QIODevice::Text);
    auto jsonConfigDoc = QJsonDocument::fromJson(inputFileConfig.readAll());
    auto jsonConfigObj = jsonConfigDoc.object();
    inputFileConfig.close();
    return jsonConfigObj;
}

void Globals::setOnConfigurationSaved(std::function<void (QWidget*, QString)> m)
{
    m_onConfigSavedMethod = m;
}
