#ifndef DRAGFRAME_H
#define DRAGFRAME_H

#include <QDrag>
#include <QFrame>
#include <QTimer>

class DragFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DragFrame(QWidget* parent);

    void restoreOriginalStyleSheet();
    void frameSelected();

    void setProperty(QString name, const QVariant& value);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent*) override;

    virtual void mousePressEvent  (QMouseEvent* event) override;
    virtual void mouseMoveEvent   (QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    bool m_dragEnabled { false };
    bool m_dragStarted { false };

    QDrag*  m_drag     { nullptr };
    QFrame* m_dragItem { nullptr };
    QString m_styleSheet;
    QTimer  m_unselectTimer;

signals:
    void dragEnteredFrame(QFrame*);
    void dragStarted(QFrame*);
    void frameDropped(QFrame*) const;
    void showContextMenu(DragFrame* source);
    void fieldSelected(DragFrame*);
    void fieldUnselected();
};

#endif // DRAGFRAME_H
