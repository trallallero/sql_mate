#include "dragframe.h"
#include "globals.h"

#include <QMouseEvent>
#include <QMimeData>
#include <QLabel>
#include <QPainter>
#include <QCheckBox>
#include <QGroupBox>
#include <QStyle>
#include <QDebug>

DragFrame::DragFrame(QWidget* parent)
    : QFrame(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    setCursor(Qt::SizeAllCursor);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    setMouseTracking(true);
    setAcceptDrops(true);
    m_unselectTimer.setInterval(Globals::selectedFieldMilliseconds());
    connect(&m_unselectTimer, &QTimer::timeout, this, [this](){
        emit fieldUnselected();
        m_unselectTimer.stop();
    });
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint&){
        emit showContextMenu(this);
    });
    m_styleSheet = styleSheet();
}

void DragFrame::restoreOriginalStyleSheet()
{
    this->setStyleSheet(m_styleSheet);
}

void DragFrame::frameSelected()
{
    m_unselectTimer.start();
}

void DragFrame::setProperty(QString name, const QVariant& value)
{
    if(name == "conditionFrame")
    {
        if(value == "enabled")
        {
            Globals::setWidgetShadow(this, this, 5);
        }
        else if(value == "disabled")
        {
            Globals::setWidgetShadow(this, this, 0);
        }
    }
    QFrame::setProperty(name.toLocal8Bit().data(), value);
    style()->unpolish(this);
    style()->polish(this);
}

bool DragFrame::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::Enter)
    {
        auto cb = qobject_cast<QCheckBox*>(obj);
        if(cb && cb->isEnabled() == false)
        {
            setCursor(Qt::ForbiddenCursor);
            QApplication::processEvents();
        }
    }
    else if(event->type() == QEvent::Leave)
    {
        setCursor(Qt::SizeAllCursor);
        QApplication::processEvents();
    }
    return QWidget::eventFilter(obj, event);
}

void DragFrame::dragEnterEvent(QDragEnterEvent* event)
{
    if(m_dragStarted)
        return;
    this->setStyleSheet(QString("background-color: #D2D6A8;")); // TODO: make this dynamic (setProperty?)
    event->setAccepted(true);
    emit dragEnteredFrame(this);
}

void DragFrame::dragLeaveEvent(QDragLeaveEvent*)
{
    this->setStyleSheet(m_styleSheet);
}

void DragFrame::mousePressEvent(QMouseEvent* event)
{
    m_unselectTimer.stop();
    emit fieldUnselected();

    if (event->button() == Qt::LeftButton)
    {
        m_dragItem    = this;
        m_dragEnabled = true;
        m_dragStarted = false;
    }
}

void DragFrame::mouseMoveEvent(QMouseEvent* event)
{    
    if(m_dragItem && m_dragEnabled && m_dragStarted == false)
    {
        m_dragStarted = true;

        auto ss     = m_dragItem->styleSheet();
        auto pixmap = m_dragItem->grab();

        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << pixmap << QPoint(event->pos());

        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", itemData);

        m_drag = new QDrag(this);
        m_drag->setMimeData(mimeData);
        m_drag->setPixmap(pixmap);
        m_drag->setHotSpot(event->pos());

        m_dragItem->setStyleSheet("background-color: #555555;"); // TODO: make this dynamic (setProperty?)

        emit dragStarted(m_dragItem);

        if (m_drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        {
            m_dragItem->close();
        }

        emit frameDropped(m_dragItem);
        m_drag->deleteLater();
        m_dragItem->setStyleSheet(ss);

        m_dragEnabled = false;
        m_dragStarted = false;
        m_dragItem    = nullptr;
    }
}

void DragFrame::mouseReleaseEvent(QMouseEvent*)
{
    m_dragEnabled = false;
    m_dragStarted = false;
    m_dragItem    = nullptr;
}

void DragFrame::mouseDoubleClickEvent(QMouseEvent*)
{
    emit fieldSelected(this);
    m_unselectTimer.start();
}
