#ifndef ELIMINAANAGRAFICA_H
#define ELIMINAANAGRAFICA_H

#include "plugininterface.h"

#include "deletequeriesviewer.h"

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class EliminaAnagrafica;
}

class EliminaAnagrafica : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    explicit EliminaAnagrafica(QWidget* parent = nullptr);
    ~EliminaAnagrafica();

    virtual void       execute         () override;
    virtual QString    getName         () const override { return "Elimina Anagrafica"; }
    virtual QString    getIconFilename () const override { return "eliminaanagraficaMenuIcon.png"; }
    virtual bool       isEnabled       () override { return true; }
    virtual PluginType getType         () const override { return PT_MainMenu; }

private slots:
    void saveConfig();
    void findValues();
    void remove();
    void enableSearchButton();

private:
    using ConnectionType = enum { CT_YIN, CT_SOLIDUS };

    Ui::EliminaAnagrafica* ui        { nullptr };
    DeleteQueriesViewer    m_deleteQueriesViewer;
    QSqlDatabase*          m_sql     { nullptr };
    QString                m_configFilename;
    QString                m_solidusConnectionName;
    QString                m_yinConnectionName;
    QString                m_mainwidgetConnectionName;
    bool                   m_uiSetup { false };
    bool                   m_onlyCreateQueries { false };
    int                    m_yinUserId;
    int                    m_solidusUserId;

    void connectObjects();
    void cleanUp();
    void resetTableWidget(QTableWidget* w);
    void setConnection(ConnectionType ct);
    void addConnectionItems();
    void loadAndApplyConfig();
    void setTenants();

    QString yinSelectEmail(bool* error);

    void yinSelectUserId();
    void yinGetValuesByEmail();
    void yinSelectOrders();

    void yinDeleteCustomerSeller();
    void yinDeleteCustomer();

    void yinDeleteOrders();

    void solidusSelectUserId();
    void solidusGetValuesByEmail();
    void solidusSelectOrders();

    void solidusDeleteYoloUserAcceptances();
    void solidusDeleteSpreeAddressesShip();
    void solidusDeleteSpreeAddressesBill();
    void solidusDeleteSpreeUsers();

    void solidusDeleteYoloInsuranceInfos();
    void solidusDeleteSpreeLineItems();
    void solidusDeleteSpreeOrders();

    void deleteFromQuery(QString query, bool failOnNoRowsAffected = true);
};

#endif // ELIMINAANAGRAFICA_H
