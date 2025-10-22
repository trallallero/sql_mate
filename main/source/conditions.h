#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "conditionfield.h"
#include "dragframe.h"
#include "conditionscontextmenu.h"
#include "conditionfieldhandler.h"

#include <QList>
#include <QMap>
#include <QGroupBox>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonObject>
#include <QCheckBox>
#include <QPushButton>
#include <QLayout>
#include <QMenu>
#include <QWidgetAction>
#include <QMouseEvent>

class Conditions : public QObject
{
    Q_OBJECT

public:
    using FieldDirection = enum { FD_UP, FD_DOWN };

    Conditions() = default;
    Conditions(QWidget* parent);

    ConditionFieldHandler* getHandler() const { return m_conditionFieldHandler; }

    void addConditionFields(QList<QGroupBox*> groupBoxes);
    void addConditionField (QList<QGroupBox*> groupBoxes, QJsonObject jo);
    void addConditionField (QJsonObject jo);

    QString            getConditions          () const;
    QStringList        getConditionNames      (bool lower, bool onlyOneArgument) const;
    QList<QJsonObject> getJsonConditions      () const;
    QVariantMap        getJsonConditionsValues() const;

    void removeConditionFields    ();
    void clearAll                 ();
    void deactivateAll            ();
    bool moveFrameTo              (QFrame* frame, QGroupBox* groupBox, int page);
    void moveCurrentField         (Conditions::FieldDirection fd);
    void uniteSelectedFields      (QStringList names, QString joinName);
    void uniteEnabledFields       (DragFrame*, QString title);

public slots:
    void dragStarted       (QFrame* frame);
    void frameDropped      (QFrame*);
    void setDropTargetFrame(QFrame* frame);

private:
    QWidget*                m_parent;
    ConditionFieldHandler*  m_conditionFieldHandler { nullptr };
    CheckBoxFieldMap        m_checkBoxFieldMap;
    FrameCheckBoxMap        m_frameCheckBoxMap;
    QList<QGroupBox*>       m_groupBoxes;
    DragFrame*              m_currentSelectedFrame  { nullptr };
    QFrame*                 m_currentDraggingFrame  { nullptr };
    QFrame*                 m_currentDropFrame      { nullptr };
    ConditionsContextMenu*  m_contextMenu           { nullptr };

    QWidget* createConditionField(
            QGroupBox*         parentGb,
            const QJsonObject& joValues,
            const QJsonObject& joConfig,
            QString            value = "",
            bool               enabled = false);

    void connectFieldObjects       (ConditionFieldData& cfd);
    void onCheckBoxTitleToggled    (DragFrame* frame, bool toggled);
    void deleteField               (QString name, QFrame* parentFrame);
    void uniteConditions           (QStringList names, QString joinName, UniteConditionsType predicate);
    void extractConditionFromFrame (DragFrame* frame, QString conditionTitle);
    QString convertConditionToLower   (QString condition) const;

private slots:
    void showContextMenu(DragFrame* frame);
    void fieldSelected  (DragFrame* frame);
    void fieldUnselected();

protected:
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void setDropTargetFrameSignal    (QFrame*);
    void dragStartedSignal           (QFrame* frame);
    void lineEditFocusRequest        ();
    void lineEditEnterPressed        ();
    void maxNumberOfConditionsReached() const;
    void checkboxDeleted             (QObject* = nullptr);
    void frameSelected               (QFrame* frame, int position, int total);
    void visibleGroups               (int);
    void showMessage                 (QString);
};

#endif // CONDITIONS_H
