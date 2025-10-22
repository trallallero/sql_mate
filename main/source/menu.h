#ifndef MENU_H
#define MENU_H

#include <QMenu>
#include <QObject>

class Menu : public QMenu
{
    Q_OBJECT

public:
    Menu(QWidget* parent = nullptr);
};

#endif // MENU_H
