#include "popupmenu.h"

#include <QTimer>
#include <QEvent>

PopupMenu::PopupMenu(QWidget* parent, QMap<int, QString> items, QString currentKey, MenuCallback callback)
    : QMenu(parent)
    , m_callback(callback)
{
    m_menuNewLineEdit.installEventFilter(this);

    auto menu = new QMenu(this);
    m_shadowMenuWidget = new ContextMenuWidget(menu, this);

    menu->setObjectName("contextMenu_0");

    for(auto& value : items.toStdMap())
    {
        if(value.second == currentKey)
        {
            auto a = new QAction(QIcon(":/images/dot.png"), value.second);
            a->setEnabled(false);
            menu->addAction(a);
        }        
        else if(value.second == "separator")
        {
            menu->addSeparator();
        }
        else if(value.second == tr("<nuovo>"))
        {
            addLineEditToMenu(menu, value.second);
        }
        else
        {
            menu->addAction(value.second);
        }
    }
    if(items.count() == 2) // means only a separator and the lineedit
        m_menuNewLineEdit.setFocus(); // save a mouse click

    connect(menu, &QMenu::triggered, this, [this](QAction* action){
        auto value = action->text();
        QTimer::singleShot(0, this, [this, value](){
            m_callback(value, false);
        });
    });
    connect(menu, &QMenu::aboutToHide, [menu](){
        menu->deleteLater();
    });

    m_shadowMenuWidget->popup();
}

bool PopupMenu::eventFilter(QObject* object, QEvent* event)
{
    if(object == &m_menuNewLineEdit && event->type() == QEvent::FocusIn)
    {
        QTimer::singleShot(0, this, [this](){
            m_menuNewLineEdit.selectAll();
        });
        return false; // lets the event continue to the edit
    }
    return false;
}

void PopupMenu::addLineEditToMenu(QMenu* menu, QString text)
{
    m_menuNewAction = new QWidgetAction(this);
    m_menuNewAction->setDefaultWidget(&m_menuNewLineEdit);
    m_menuNewLineEdit.setText(text);
    menu->addAction(m_menuNewAction);

    connect(&m_menuNewLineEdit, &QLineEdit::returnPressed, this, [this, menu](){
        QTimer::singleShot(0, this, [this](){
            m_callback(m_menuNewLineEdit.text(), false);
        });
        menu->close();
    });
}
