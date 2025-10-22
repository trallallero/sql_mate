#ifndef JSONVIEWER_H
#define JSONVIEWER_H

#include "plugininterface.h"

#include <QDialog>

namespace Ui {
class JsonViewer;
}

class JsonViewer : public QDialog, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(PluginInterface)

public:
    JsonViewer();
    ~JsonViewer();

    virtual QString    getName            () const override;
    virtual void       execute            () override;
    virtual QString    getIconFilename    () const override  { return "jsonviewerMenuIcon.png"; }
    virtual bool       isEnabled          () override;
    virtual PluginType getType            () const override { return PT_TableWidget; }

private slots:
    void pushButtonClose_clicked();
    void toolButtonCopy_clicked();

private:
    Ui::JsonViewer* ui {nullptr};
    bool            m_uiSetup {false};
    QByteArray      m_data;

    void connectObjects();
};

#endif // JSONVIEWER_H
