#ifndef POPUPMENU_H
#define POPUPMENU_H

#include "contextmenuwidget.h"

#include <QMenu>
#include <QLineEdit>
#include <QWidgetAction>

using MenuCallback = std::function<void(QString, bool)>;

class PopupMenu : public QMenu
{
    Q_OBJECT
public:
    explicit PopupMenu(QWidget* parent, QMap<int, QString> items, QString currentKey, MenuCallback callback);

protected:
    bool eventFilter(QObject* object, QEvent* event);

private:
    ContextMenuWidget*  m_shadowMenuWidget { nullptr };
    MenuCallback        m_callback;
    QWidgetAction*      m_menuNewAction { nullptr };
    QLineEdit           m_menuNewLineEdit;

    void addLineEditToMenu(QMenu* menu, QString text);
};

#endif // POPUPMENU_H
