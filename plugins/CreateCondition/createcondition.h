#ifndef CREATECONDITION_H
#define CREATECONDITION_H

#include "plugininterface.h"

#include <QDialog>

namespace Ui {
class CreateCondition;
}

class CreateCondition : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    CreateCondition();
    ~CreateCondition();

    virtual QString    getName         () const override;
    virtual void       execute         () override;
    virtual QString    getIconFilename () const override  { return "createConditionMenuIcon.png"; }
    virtual bool       isEnabled       () override;
    virtual PluginType getType         () const override { return PT_TableWidget; }

private slots:
    void pushButtonCancel_clicked();
    void pushButtonCreate_clicked();
    void checkBoxIN_toggled(bool checked);
    void checkBoxLIKE_toggled(bool checked);
    void checkBoxNULL_toggled(bool checked);

private:
    Ui::CreateCondition* ui        {nullptr};
    bool                 m_uiSetup {false};
    QString              m_conditions;

    void connectObjects();
    void setConditionFromTitle(QString filename, QString title);
    void uncheckRadioButtons();
};

#endif // CREATECONDITION_H
