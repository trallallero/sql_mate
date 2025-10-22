#include "dialog.h"

#include <QMouseEvent>

void Dialog::mouseMoveEvent(QMouseEvent* event)
{
    if(parent() && m_cursorStartPos != QPoint{0,0})
    {
        const auto modifiers = event->modifiers();
        if(modifiers & Qt::AltModifier && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
        {
            auto parentWidget = qobject_cast<QDialog*>(parent());
            if(parentWidget)
            {
                m_wasMoved = true;
                auto eventPos = mapToGlobal(event->pos());

                parentWidget->move(m_widgetStartPos.x() + (eventPos.x() - m_cursorStartPos.x()),
                                   m_widgetStartPos.y() + (eventPos.y() - m_cursorStartPos.y()));
            }
        }
    }
}

void Dialog::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton && parent())
    {
        const auto modifiers = event->modifiers();
        if(modifiers & Qt::AltModifier && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
        {
            auto parentWidget = qobject_cast<QDialog*>(parent());
            if(parentWidget)
            {
                parentWidget->setCursor(Qt::SizeAllCursor);
                m_cursorStartPos = mapToGlobal(event->pos());
                m_widgetStartPos = parentWidget->pos();
            }
        }
    }
}

void Dialog::mouseReleaseEvent(QMouseEvent*)
{
    m_cursorStartPos = {0, 0};
    if(m_wasMoved && parent())
    {
        m_wasMoved = false;

        auto parentWidget = qobject_cast<QDialog*>(parent());
        if(parentWidget)
            parentWidget->setCursor(Qt::ArrowCursor);
    }
}
