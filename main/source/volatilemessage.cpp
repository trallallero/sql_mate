#include "volatilemessage.h"
#include "ui_volatilemessage.h"
#include "globals.h"

#include <limits.h>

VolatileMessage* VolatileMessage::m_instance = nullptr;

VolatileMessage::VolatileMessage(QWidget* parent, QString message)
    : QFrame(parent)
    , ui(new Ui::VolatileMessage)
    , m_message(message)
{
    ui->setupUi(this);
    ui->labelMessage->setText(m_message);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);

    Globals::setWidgetShadow(parent, this, 5);
    move(x() + parent->width() / 2 - width(), y() + parent->height() - height() * 2);
}

VolatileMessage::~VolatileMessage()
{
    delete ui;
}

void VolatileMessage::show(bool infinite)
{
    stop();

    connect(&m_timer, &QTimer::timeout, this, [this](){
        m_instance = nullptr;
        close();
        deleteLater();
    });

    m_instance = this;
    if(infinite)
        m_timer.start(INT_MAX);
    else
        m_timer.start(1000 + (m_message.length() * 20));
    return QWidget::show();
}

void VolatileMessage::mousePressEvent(QMouseEvent*)
{
    stop();
}

void VolatileMessage::stop()
{
    if (m_instance)
    {
        m_instance->m_timer.stop();
        delete m_instance;
        m_instance = nullptr;
    }
}
