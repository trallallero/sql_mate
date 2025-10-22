#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "sql.h"
#include "conditions.h"
#include "pluginshandler.h"
#include "tablewidget.h"
#include "globals.h"

#include <QMainWindow>
#include <QList>
#include <QCheckBox>
#include <QSqlDatabase>
#include <QMenu>
#include <QTableWidget>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent (QDragMoveEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
    virtual void dropEvent     (QDropEvent*) override;
    virtual void changeEvent   (QEvent* event) override;

private slots:
    void checkBoxZeroDefaultProfile_toggled(bool checked);

    void on_pushButtonSearch_clicked();
    void on_pushButtonFF_clicked();
    void on_pushButtonExportResult_clicked();
    void on_pushButtonViewQuery_clicked();
    void on_pushButtonSetStyle_clicked();
    void on_pushButtonZeroProfiles_clicked();
    void on_pushButtonZeroSchemas_clicked();
    void on_pushButtonZeroSaveProfile_clicked();
    void on_pushButtonSwitchView_clicked();
    void on_pushButtonZeroSelectProfile_clicked();
    void on_pushButtonZeroSelectSchema_clicked();
    void on_pushButtonZeroSaveSchema_clicked();
    void on_checkBoxSelectTenants_toggled(bool checked);
    void on_checkBoxZeroDefaultSchema_toggled(bool checked);
    void on_comboBoxConnection_currentIndexChanged(const QString& arg1);
    void on_comboBoxLimitSearch_currentIndexChanged(int);
    void on_toolButtonReset_clicked();
    void on_toolButtonDeactivate_clicked();
    void on_toolButtonAddCondition_clicked();
    void on_toolButtonDeleteAll_clicked();
    void on_toolButtonCondUp_clicked();
    void on_toolButtonCondDown_clicked();
    void on_tableWidgetResult_itemSelectionChanged();

    void setProfile(QString profile, bool isDefault = false, bool force = false);
    void showVolatileMessage(const QString message, bool infinite = false);
    void addCondition(QJsonObject jo);
    void showConnections();
    void showAbout();
    void sqlResultReady(SqlResultType result);
    void pluginExportResult(QWidget* parent, QTableWidget* tw, ViewMode vm);
    void setProfileQueryLimit();
    void queryByTenant(QString tenant, QString& query);

private:
    Ui::MainWindow* ui;

    QList<QCheckBox*>   m_tenantsCheckBoxes;
    QWidget*            m_lastFocusWidget {nullptr};
    QList<QString>      m_selectedFields;
    Sql                 m_sql;
    Conditions          m_conditions;
    QString             m_currentQuery;
    PluginsHandler      m_pluginsHandler;
    SqlResultType       m_sqlResult;
    TenantQueryMap      m_tenantQueryMap;
    ViewMode            m_currentViewMode       { ViewMode::VM_HORIZONTAL };
    bool                m_showConnectionMessage { true };
    bool                m_setupFinished         { false };
    bool                m_dropAccepted          { false };
    bool                m_connectedToDb         { false };
    QGroupBox*          m_currentDropGroupBox   { nullptr };
    QGroupBox*          m_currentDragGroupBox   { nullptr };
    QFrame*             m_currentDropFrame      { nullptr };
    QRect               m_highlightedRect;
    QTranslator         m_translator;

    static MainWindow*  m_instance;

    QRect       targetSquare(const QPoint& position) const;
    void        createMenu();
    void        connectObjects();
    QStringList getEnabledTenants() const;
    void        resetTableWidget();
    void        addTenantCheckBoxes();
    void        tenant_checked(bool toggled);
    void        applyProfile();
    void        setProfileTenantsChecked();
    void        setProfileConnection(bool force);
    void        setupPlugins();
    void        enableWidgetObjects(bool enable, bool canBeCanceled);
    void        finishSetup();
    void        loadSchema();
    void        switchTranslator(QTranslator& translator, const QString& filename);
    void        setViewModeTextToButton();
    QJsonObject getProfileSchemaJsonValues() const;

    static void onConfigurationSaved(QWidget* w, QString message);
};
#endif // MAINWINDOW_H
