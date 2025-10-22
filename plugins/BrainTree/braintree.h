#ifndef BRAINTREE_H
#define BRAINTREE_H

#include "plugininterface.h"

#include <QDialog>

namespace Ui {
class BrainTree;
}

enum ValueType
{
    VT_SUBSCRIPTION,
    VT_EMAIL
};

class BrainTree : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    BrainTree();
    ~BrainTree();

    virtual void       execute         () override;
    virtual QString    getName         () const override { return "Braintree transactions"; }
    virtual QString    getIconFilename () const override { return "brainTreeMenuIcon.png"; }
    virtual bool       isEnabled       () override;
    virtual PluginType getType         () const override { return PT_TableWidget; }

private slots:
    void pushButtonClose_clicked();
    void pushButtonExport_clicked();
    void pushButtonSave_clicked();

private:
    Ui::BrainTree* ui {nullptr};

    bool    m_transactionsShown {false};
    bool    m_uiSetup           {false};

    static QString subscriptionQuery;

    void        connectObjects();
    void        resetTableWidget();
    ValueType   valueType(QString value) const;
    QStringList getSubscriptionsByEmail(QString email) const;

    bool hasConnectionData() const;
    void executePython();

    void saveConfig();
    void loadConfig();
};

#endif // BRAINTREE_H
