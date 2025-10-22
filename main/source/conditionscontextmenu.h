#ifndef CONDITIONSCONTEXTMENU_H
#define CONDITIONSCONTEXTMENU_H

#include "conditionfield.h"
#include "dragframe.h"
#include "contextmenuwidget.h"

#include <QMenu>
#include <QLineEdit>
#include <QWidgetAction>
#include <QCheckBox>

class ConditionsContextMenu : public QMenu
{
    Q_OBJECT

public:
    ConditionsContextMenu(QWidget* parent);

    void showContextMenu(DragFrame* frame, const ConditionField& field, QMap<QCheckBox*, ConditionField>& cbFieldMap);

signals:
    void uniteFields (DragFrame* frame, QString title);
    void extractField(DragFrame* frame, QString title);
    void renameField (DragFrame* frame, QString title);

private:
    ContextMenuWidget* m_shadowMenuWidget  { nullptr };
    QWidgetAction*     m_uniteTitleAction  { nullptr };
    QWidgetAction*     m_renameTitleAction { nullptr };
    DragFrame*         m_frame             { nullptr };
    QMenu              m_contextMenu;
    QMenu              m_extractFieldMenu;
    QMenu              m_uniteFieldsMenu;
    QMenu              m_renameFieldMenu;
    QLineEdit          m_uniteLineEdit;
    QLineEdit          m_renameLineEdit;

    QWidget* createWidget(QLineEdit* lineEdit, void (ConditionsContextMenu::*method)(DragFrame*, QString));
};

#endif // CONDITIONSCONTEXTMENU_H
