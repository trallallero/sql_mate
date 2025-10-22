#include "widgetsizetool.h"
#include "globals.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>

SizeToolButton::SizeToolButton(QWidget* parent)
    : QToolButton(parent)
{
    setIcon(QIcon(":/images/maximize.png"));
    setCursor(Qt::SizeAllCursor);
}

void SizeToolButton::setMainWidget(QWidget* w)
{
    m_mainWidget = w;
}

void SizeToolButton::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_startPoint = event->pos();
        m_wasMoved = false;
    }
}

void SizeToolButton::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        m_wasMoved = true;

        const auto modifiers = event->modifiers();
        if(modifiers & Qt::AltModifier && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
        {
            m_mainWidget->move(m_mainWidget->x() + event->pos().x() - width()  / 2,
                               m_mainWidget->y() + event->pos().y() - height() / 2);
        }
        else
        {
            m_mainWidget->resize(m_mainWidget->width()  + event->pos().x() - width()  / 2,
                                 m_mainWidget->height() + event->pos().y() - height() / 2);
        }
    }
}

void SizeToolButton::mouseReleaseEvent(QMouseEvent*)
{
    if(m_wasMoved == false)
    {
        if(m_mainWidget->isMaximized())
            emit showAsNormal();
        else
            emit showAsMaximized();
    }
}

//////////////////////////////////////////////////////

WidgetSizeTool::WidgetSizeTool(QWidget* mainWidget, QWidget* parent, bool addLeftSpacer)
    : QGroupBox(parent)
    , m_mainWidget(mainWidget)
    , m_toolButton(new SizeToolButton(this))
{
    auto parentHLayout = qobject_cast<QHBoxLayout*>(parent->layout());
    if(parentHLayout == nullptr)
    {
        parentHLayout = new QHBoxLayout;
        parentHLayout->setContentsMargins(0, 0, 0, 0);
        parent->setLayout(parentHLayout);
    }

    setObjectName("groupBoxZeroWidgetSizeTool");

    auto hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    auto lbl = new QLabel(tr("Salva"), this);
    hLayout->insertWidget(hLayout->count(), lbl);

    m_tb = new QToolButton(this);
    m_tb->setMaximumSize(16, 16);
    m_tb->setCursor(Qt::PointingHandCursor);

    hLayout->insertWidget(hLayout->count(), m_tb);
    hLayout->insertWidget(hLayout->count(), m_toolButton);

    m_toolButton->setMainWidget(mainWidget);

    connect(m_toolButton, &SizeToolButton::showAsNormal   , this, &WidgetSizeTool::showAsNormal);
    connect(m_toolButton, &SizeToolButton::showAsMaximized, this, &WidgetSizeTool::showAsMaximized);

    if(addLeftSpacer)
        parentHLayout->insertItem(0, new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    parentHLayout->insertItem(parentHLayout->count(), new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    parentHLayout->insertWidget(parentHLayout->count(), this);

    connect(m_tb, &QToolButton::clicked, this, [this](){
        QByteArray geom    = m_mainWidget->saveGeometry();
        QString    objName = m_mainWidget->objectName();

        if(objName.isEmpty())
        {   // in case of Globals::createShadowedGui, the widget is a QDialog of the mainWidget
            auto dialog = m_mainWidget->findChild<QDialog*>();
            if(dialog)
                objName = dialog->objectName();
        }

        auto configObj  = Globals::globalConfigurationObject();
        configObj[objName + "Geometry"] = QString(geom.toBase64());
        if(m_mainWidget->isMaximized())
            configObj[objName + "State"] = "maximized";
        else
            configObj[objName + "State"] = "normal";

        Globals::saveGlobalConfigurationObject(configObj, m_mainWidget, tr("Aspetto finestra salvato"));
    }, Qt::QueuedConnection);

    QTimer::singleShot(0, this, [this](){
        auto objName = m_mainWidget->objectName();
        if(objName.isEmpty())
        {   // in case of Globals::createShadowedGui, the widget is a QDialog of the mainWidget
            auto dialog = m_mainWidget->findChild<QDialog*>();
            if(dialog)
                objName = dialog->objectName();
        }

        auto configObj = Globals::globalConfigurationObject();
        auto geom = configObj[objName + "Geometry"].toString();
        auto ba = QByteArray::fromBase64(geom.toLocal8Bit());
        m_mainWidget->restoreGeometry(ba);

        if(m_mainWidget->isVisible())
        {
            auto state = configObj[objName + "State"].toString();
            if(state == "maximized")
                showAsMaximized();
            else
                showAsNormal();
        }
    });
}

void WidgetSizeTool::showAsMaximized()
{
    auto dialog = m_mainWidget->findChild<QDialog*>();
    if(dialog)
    {
        Globals::setWidgetShadow(m_mainWidget, dialog, 0);
        dialog->showMaximized();
    }
    m_mainWidget->layout()->setMargin(0);
    m_mainWidget->showMaximized();
    m_toolButton->setIcon(QIcon(":/images/minimize.png"));
}

void WidgetSizeTool::showAsNormal()
{
    auto dialog = m_mainWidget->findChild<QDialog*>();
    if(dialog)
    {
        Globals::setWidgetShadow(m_mainWidget, dialog, 8);
        dialog->showNormal();
    }
    m_mainWidget->layout()->setMargin(11);
    m_mainWidget->showNormal();
    m_toolButton->setIcon(QIcon(":/images/maximize.png"));
}
