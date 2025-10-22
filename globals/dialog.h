#ifndef DIALOG_H
#define DIALOG_H

#if defined GLOBALSLIB_LIBRARY
    #define GLOBALS_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GLOBALS_LIB_EXPORT Q_DECL_IMPORT
#endif

#include <QDialog>
#include <QObject>
#include <QWidget>

class GLOBALS_LIB_EXPORT Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget* parent = nullptr) : QDialog(parent) {}

protected:
    virtual void mouseMoveEvent   (QMouseEvent* event) override;
    virtual void mousePressEvent  (QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent*      ) override;

private:
    QPoint m_cursorStartPos;
    QPoint m_widgetStartPos;
    bool   m_wasMoved { false };
};


#endif // DIALOG_H
