#ifndef VOLATILEMESSAGE_H
#define VOLATILEMESSAGE_H

#include <QFrame>
#include <QString>
#include <QTimer>

namespace Ui {
class VolatileMessage;
}

class VolatileMessage : public QFrame
{
    Q_OBJECT

public:
    explicit VolatileMessage(QWidget* parent, QString message);
    ~VolatileMessage();

    void show(bool infinite = false);
    static void stop();

protected:
    virtual void mousePressEvent(QMouseEvent*) override;

private:
    Ui::VolatileMessage* ui;
    QString              m_message;
    QString              m_styleSheets;
    QTimer               m_timer;

    static VolatileMessage* m_instance;
};

#endif // VOLATILEMESSAGE_H
