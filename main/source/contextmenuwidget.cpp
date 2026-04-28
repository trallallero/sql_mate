#include <QApplication>
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include "contextmenuwidget.h"

#include "globals.h"

ContextMenuWidget::ContextMenuWidget(QMenu* menu, QWidget* parent)
    : QWidget(parent)
    , m_contextMenu(menu)
{
    setWindowFlags(windowFlags() | Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto lo = new QHBoxLayout();
    lo->setMargin(5);
    setLayout(lo);
    lo->addWidget(m_contextMenu);

    Globals::setWidgetShadow(this, m_contextMenu, 5);

    connect(m_contextMenu, &QMenu::aboutToHide, this, &QWidget::hide);
    connect(m_contextMenu, &QMenu::triggered  , this, &QWidget::hide);
}

void ContextMenuWidget::popup()
{
    move(QCursor::pos());
    m_contextMenu->setVisible(true);
    QTimer::singleShot(0, this, [this]() {
        QScreen* screen = this->window()->windowHandle()->screen();
        QRect screenGeom = screen->availableGeometry();
        QPoint newPos = this->pos();

        if (this->y() + this->height() > screenGeom.bottom())
            newPos.setY(screenGeom.bottom() - this->height());

        if (this->x() + this->width() > screenGeom.right())
            newPos.setX(screenGeom.right() - this->width());

        this->move(newPos);
    });
    show();
}
