#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <QPushButton>
#include <QObject>

class PushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PushButton(QWidget* parent = nullptr);

    void setProperty(QString name, const QVariant& value);

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
};

#endif // PUSHBUTTON_H
