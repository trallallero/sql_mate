#ifndef CONDITIONFIELDHANDLER_H
#define CONDITIONFIELDHANDLER_H

#include "conditionfield.h"
#include "dragframe.h"

#include <QStringList>
#include <QLineEdit>
#include <QGroupBox>
#include <QJsonObject>
#include <QPair>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>

using OrderedFields       = QList<QList<QJsonObject>>;
using UniteConditionsType = std::function<bool (std::pair<QFrame* const, QCheckBox*>&, QStringList)>;
using CheckBoxFieldMap    = QMap<QCheckBox*, ConditionField>;
using FrameCheckBoxMap    = QMap<QFrame*, QCheckBox*>;
using ConditionFieldData  =

// TODO: make this a class with better names and move the logic (checked/unchecked-enabled/disabled) of conditions.cpp in here
//       Or move the QMap<QString, QCheckBox*> cbs in a separate class
struct ConditionFieldData
{
    ConditionFieldData() {}

    QWidget*                  widget  { nullptr };
    DragFrame*                frame   { nullptr };
    QCheckBox*                cb      { nullptr };
    QList<QLineEdit*>         les     { nullptr };
    QLineEdit*                sle     { nullptr }; // single line edit, when there are checkboxes
    QMap<QString, QCheckBox*> cbs     {};
    QPushButton*              bt      { nullptr };
    ConditionField            field   {};
    bool                      enabled { false };
};
///////////////////////////////////////

class ConditionFieldHandler : public QObject
{
    Q_OBJECT

public:
    ConditionFieldHandler(
            QObject*           parent,
            QList<QGroupBox*>& groupBoxes,
            CheckBoxFieldMap&  checkBoxFieldMap,
            FrameCheckBoxMap&  frameCheckBoxMap);

    ConditionField getFieldByFrame                       (QFrame* frame) const;
    void           setFieldByFrame                       (QFrame* frame, ConditionField field);
    void           swapFields                            (QFrame* frame1, QFrame* frame2);
    void           setFieldCoordinates                   (QFrame* frame, int page, int position);
    OrderedFields  getOrderedFields                      () const;
    int            getFieldsCountPerPage                 (int page) const;
    void           renameField                           (DragFrame* frame, QString title);
    void           updateLayoutSize                      ();
    void           updateFieldsPositionAfterRemovingField(int page);
    void           updateFieldsPositionAfterMovingField  (int page);
    void           insertWidgetInLayout                  (QLayout* lo, QWidget* w);
    void           deleteField                           (QString name, QFrame* parentFrame);
    int            fieldsPerPage                         (int page) const;

    static QStringList checkBoxesConditionNames();

    static ConditionFieldData createConditionField(
        QGroupBox* parentGb,
        const QJsonObject& joValues,
        const QJsonObject& joConfig,
        QString value,
        bool enabled);

signals:
    void maxNumberOfConditionsReached() const;
    void showMessage                 (QString);
    void visibleGroups               (int);

private:
    QList<QGroupBox*>& m_groupBoxes;
    CheckBoxFieldMap&  m_checkBoxFieldMap;
    FrameCheckBoxMap&  m_frameCheckBoxMap;

    OrderedFields  getInitializedOrderedFields() const;

    static QVector<QPair<QString, QString>> checkBoxesConditions;

    static QPair<DragFrame*, QVBoxLayout*> createFieldFrame(
            ConditionFieldData& cfd,
            QGroupBox* parentGb,
            QString objName,
            QString fieldName);

    static QLineEdit* createFieldLineEdit(
            QFrame* frame,
            QString userText,
            QString objName,
            const QJsonObject& joConfig,
            QString fieldName);

    static QGroupBox* createFieldCheckBoxes(
            ConditionFieldData& cfd,
            QFrame* frame,
            QString name,
            QString objName,
            const QJsonObject& joConfig);

    static bool setFieldEnabled(
            QCheckBox* cb,
            QString objName,
            const QJsonObject& joConfig,
            bool enabled);

    static QPushButton* createDeleteConditionButton();
};

#endif // CONDITIONFIELDHANDLER_H
