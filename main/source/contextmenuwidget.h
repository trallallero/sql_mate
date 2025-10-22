#ifndef CONTEXTMENUWIDGET_H
#define CONTEXTMENUWIDGET_H

#include <QMenu>

class ContextMenuWidget : public QWidget
{
public:
    ContextMenuWidget(QMenu* menu, QWidget* parent = nullptr);

    void popup();

private:
    QMenu* m_contextMenu { nullptr };
};

#endif // CONTEXTMENUWIDGET_H
