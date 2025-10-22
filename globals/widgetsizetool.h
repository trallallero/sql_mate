#ifndef WIDGETSIZETOOL_H
#define WIDGETSIZETOOL_H

#if defined GLOBALSLIB_LIBRARY
    #define GLOBALS_LIB_EXPORT Q_DECL_EXPORT
#else
    #define GLOBALS_LIB_EXPORT Q_DECL_IMPORT
#endif

#include <QGroupBox>
#include <QToolButton>
#include <QAction>
#include <QObject>

class SizeToolButton;

class GLOBALS_LIB_EXPORT WidgetSizeTool : public QGroupBox
{
    Q_OBJECT

public:
    WidgetSizeTool(QWidget* mainWidget, QWidget* parent, bool addLeftSpacer = true);

private:
    QWidget*        m_mainWidget { nullptr };
    SizeToolButton* m_toolButton { nullptr };
    QToolButton*    m_tb         { nullptr };

    void showAsMaximized();
    void showAsNormal();
};

class SizeToolButton : public QToolButton
{
    Q_OBJECT

public:
    SizeToolButton(QWidget* parent);

    void setMainWidget(QWidget* w);

protected:
    virtual void mousePressEvent  (QMouseEvent* event) override;
    virtual void mouseMoveEvent   (QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent*      ) override;

signals:
    void showAsMaximized();
    void showAsNormal();

private:
    QPoint   m_startPoint;
    QWidget* m_mainWidget{ nullptr };
    bool     m_wasMoved  { false };
};

#endif // WIDGETSIZETOOL_H
