#ifndef SELECTFROMTABLE_H
#define SELECTFROMTABLE_H

#include "plugininterface.h"
#include "globals.h"

#include <QDialog>
#include <QSqlQuery>

namespace Ui {
class SelectFromTable;
}

class SelectFromTable : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    SelectFromTable();
    ~SelectFromTable();

    virtual void       execute         () override;
    virtual QString    getName         () const override { return "Select From Table"; }
    virtual QString    getIconFilename () const override { return "selectFromTableMenuIcon.png"; }
    virtual bool       isEnabled       () override;
    virtual PluginType getType         () const override { return PT_TableWidget; }

private slots:
    void pushButtonClose_clicked();
    void pushButtonExport_clicked();
    void pushButtonSwitchView_clicked();
    void sqlResult(SqlResultType result, int rowsAffected);

private:
    Ui::SelectFromTable* ui                {nullptr};
    bool                 m_uiSetup         {false};
    SqlResultType        m_sqlResult;

    void connectObjects();

    static QString selectQuery;
};

#endif // SELECTFROMTABLE_H
