#ifndef ADDTOCONDITION_H
#define ADDTOCONDITION_H

#include "plugininterface.h"

#include <QDialog>

namespace Ui {
class AddToCondition;
}

class AddToCondition : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    AddToCondition();
    ~AddToCondition();

    virtual QString    getName            () const override;
    virtual void       execute            () override;
    virtual QString    getIconFilename    () const override  { return "addToConditionMenuIcon.png"; }
    virtual bool       isEnabled          () override;
    virtual PluginType getType            () const override { return PT_TableWidget; }

private slots:
    void pushButtonCancel_clicked();
    void pushButtonOk_clicked();
    void checkBoxNULL_toggled(bool checked);
    void checkBoxLIKE_toggled(bool checked);

private:
    Ui::AddToCondition* ui        {nullptr};
    bool                m_uiSetup {false};

    void connectObjects();
};

#endif // ADDTOCONDITION_H
