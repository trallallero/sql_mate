#include "pushbutton.h"

#include <QGraphicsDropShadowEffect>
#include <QVariant>
#include <QStyle>

PushButton::PushButton(QWidget* parent)
    : QPushButton(parent)
{
//    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
//    effect->setBlurRadius(3);
//    effect->setXOffset(3);
//    effect->setYOffset(3);
//    effect->setColor(QColor::fromRgb(60, 60, 60));
//    setGraphicsEffect(effect);
}

void PushButton::setProperty(QString name, const QVariant& value)
{
    if(name == "conditionButton")
    {
        if(value == "normal")
        {
        }
        else if(value == "pressed")
        {

        }
    }
    QPushButton::setProperty(name.toLocal8Bit().data(), value);
    style()->unpolish(this);
    style()->polish(this);
}

void PushButton::mousePressEvent(QMouseEvent* e)
{
//    setGraphicsEffect(nullptr);
//    move(x() + 3, y() + 3);
    QPushButton::mousePressEvent(e);
}

void PushButton::mouseReleaseEvent(QMouseEvent* e)
{
//    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
//    effect->setBlurRadius(3);
//    effect->setXOffset(3);
//    effect->setYOffset(3);
//    effect->setColor(QColor::fromRgb(60, 60, 60));
//    setGraphicsEffect(effect);
    QPushButton::mouseReleaseEvent(e);
}
