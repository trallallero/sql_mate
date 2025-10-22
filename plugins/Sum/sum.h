#ifndef SUM_H
#define SUM_H

#include "plugininterface.h"

#include <QDialog>

namespace Ui {
class Sum;
}

class Sum : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    Sum();
    ~Sum();

    virtual QString    getName            () const override;
    virtual void       execute            () override;
    virtual QString    getIconFilename    () const override  { return "sumMenuIcon.png"; }
    virtual bool       isEnabled          () override;
    virtual PluginType getType            () const override { return PT_TableWidget; }

private slots:
    void pushButtonClose_clicked();
    void toolButtonCopy_clicked();

private:
    Ui::Sum* ui {nullptr};
    bool        m_uiSetup {false};

    void connectObjects();

    bool isNumber(std::string str);
};

#endif // SUM_H
