#ifndef GLOBALS_H
#define GLOBALS_H

#if defined GLOBALSLIB_LIBRARY
    #define GLOBALS_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GLOBALS_LIB_EXPORT Q_DECL_IMPORT
#endif

#include "dialog.h"

#include <QApplication>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QUrl>
#include <QRegularExpression>
#include <QDialog>
#include <QHBoxLayout>

using SqlResultType  = QList<QMap<QString, QString>>;
using TenantQueryMap = QMap<QString, QString>;

Q_DECLARE_METATYPE(SqlResultType);
Q_DECLARE_METATYPE(TenantQueryMap);
Q_DECLARE_METATYPE(QVector<int>);

enum ViewMode
{
    VM_VERTICAL,
    VM_HORIZONTAL
};

class GLOBALS_LIB_EXPORT Globals
{
public:
    static void setApplicationStyleSheet(QApplication* app);

    static QString templateProfileName() { return m_templateDirectory; }
    static QString defaultDirectory   () { return m_defaultDirectory;  }

    static void setProfile(const QString profile) { m_profile = profile; }
    static void setSchema (const QString schema)  { m_schema  = schema; }

    static QString  currentProfile () { return m_profile; }
    static QString  currentSchema  () { return m_schema; }

    static void     setProfileToDefault     (const QString profile);
    static QString  getProfileSetToDefault  ();
    static bool     isCurrentProfileDefault () { return currentProfile() == getProfileSetToDefault(); }

    static void     setSchemaToDefault     (const QString schema);
    static QString  getSchemaSetToDefault  ();
    static bool     isCurrentSchemaDefault () { return currentSchema().isEmpty() == false && currentSchema() == getSchemaSetToDefault(); }

    static QString profilesDirectory() { return m_profilesDirectory; }
    static QString configDirectory  () { return m_configDirectory;   }
    static QString pluginsDirectory () { return m_pluginsDirectory;  }

    static QString currentProfileDirectory () { return profileDirectory(m_profile); }
    static QString currentSchemaDirectory  () { return profileDirectory(m_profile) + m_schema + (m_schema.isEmpty() ? "" : "/"); }

    static QString filenameProfileQuery          () { return currentProfileDirectory() + m_filenameQuery;           }
    static QString filenameProfileConditionFields() { return currentProfileDirectory() + m_filenameConditionFields; }
    static QString filenameProfileConfig         ();
    static QString filenameProfileSchemas        () { return currentProfileDirectory() + m_filenameSchemas;         }
    static QString filenameProfileTenantsQuery   () { return currentProfileDirectory() + m_filenameTenantsQuery;    }
    static QString filenameProfileReadMe         () { return currentProfileDirectory() + m_filenameProfileReadMe;   }

    static QString filenameConfigSelectedFields () { return configDirectory() + m_filenameSelectedFields; }
    static QString filenameConfigStyleSheet     () { return configDirectory() + m_filenameStyleSheet;     }
    static QString filenameConfigConfig         () { return configDirectory() + m_filenameConfig;         }
    static QString filenameConfigConnections    () { return configDirectory() + m_filenameConnections;    }

    static QString filenameTenantsQuery         () { return m_filenameTenantsQuery; }

    static QJsonObject profileConfigurationObject();
    static QJsonObject globalConfigurationObject();
    static QJsonObject profileSchemasObject();

    static void saveGlobalConfigurationObject(QJsonObject& configObj, QWidget* w, QString message = {});

    // -------------
    static QString profileTemplateDirectory      ()                { return profilesDirectory() + templateProfileName(); }
    static QString profileDirectory              (QString profile) { return profilesDirectory() + profile + "/"; }
    static QString schemaDirectory               (QString profile, QString schema) { return profileDirectory(profile) + schema + "/"; }

    static QString filenameProfileQuery          (QString profile) { return profileDirectory(profile) + m_filenameQuery;           }
    static QString filenameProfileConditionFields(QString profile) { return profileDirectory(profile) + m_filenameConditionFields; }
    static QString filenameProfileConfig         (QString profile) { return profileDirectory(profile) + m_filenameConfig;          }

    static QString filenameProfileDefaultConfig  () { return currentProfileDirectory() + defaultDirectory() + baseConfigFilename(); }

    static QString filenameSchemaConditionFields () { return currentSchemaDirectory() + m_filenameConditionFields; }
    static QString filenameSchemaConfig          () { return currentSchemaDirectory() + m_filenameConfig; }

    static QString baseConfigFilename            () { return m_filenameConfig; }

    static QMap<QString, QString> getAllProfileConditionsMap(QString profile);

    static auto numberOfGroups               () { return m_numberOfGroups;                                   }
    static auto maxNumberOfConditionsPerGroup() { return m_maxNumberOfConditionsPerGroup;                    }
    static auto maxNumberOfConditions        () { return maxNumberOfConditionsPerGroup() * numberOfGroups(); }
    static auto selectedFieldMilliseconds    () { return m_selectedFieldMilliseconds;                        }
    static auto sqlConnectTimeout            () { return m_sqlConnectTimeout;                                }

    static QString valuesSeparator()     { return m_valuesSeparator;     }
    static QString newConnectionString() { return m_newConnectionString; }

    static QString applicationName() { return m_applicationName; }
    static QString applicationTitle() { return applicationName() + " - " + currentProfile() + " - " + currentSchema(); }

    // TODO: really crypt/decrypt with a private key
    static QString getCrypted  (QString text) { return QByteArray(text.toLocal8Bit()).toBase64(); }
    static QString getDecrypted(QString text) { return QByteArray().fromBase64(text.toLocal8Bit()); }

    static bool copyPath(QString src, QString dst);

    static QPair<QString, QString> getTableNameAndAcronym(QString title, QString tenant, QString query);

    static void setWidgetShadow(QWidget* parent, QWidget* w, int shadow);

    static Dialog* createShadowedGui(QDialog* parent, bool isPlugin = false);

    static QColor getAlternateColor(int& index, bool increaseIndex = true);

private:
    friend class MainWindow;

    static QString  m_templateDirectory;
    static QString  m_defaultDirectory;
    static QString  m_profilesDirectory;
    static QString  m_configDirectory;
    static QString  m_pluginsDirectory;
    static QString  m_filenameQuery;
    static QString  m_filenameConditionFields;
    static QString  m_filenameConfig;
    static QString  m_filenameSchemas;
    static QString  m_filenameTenantsQuery;
    static QString  m_filenameSelectedFields;
    static QString  m_filenameStyleSheet;
    static QString  m_filenameConnections;
    static QString  m_filenameProfileReadMe;
    static QString  m_applicationName;
    static QString  m_newConnectionString;
    static QString  m_valuesSeparator;
    static QString  m_profile;
    static QString  m_schema;
    static int      m_selectedFieldMilliseconds;
    static int      m_sqlConnectTimeout;
    static int      m_numberOfGroups;
    static int      m_maxNumberOfConditionsPerGroup;

    static std::function<void(QWidget*, QString)> m_onConfigSavedMethod;

    static void initialize();

    static QJsonObject getJsonObject(QString file);

    static void setOnConfigurationSaved(std::function<void (QWidget*, QString)> m);
};

#endif // GLOBALS_H
