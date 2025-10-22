#ifndef NEWCONDITION_H
#define NEWCONDITION_H

#include <QDialog>
#include <QCheckBox>
#include <QMap>
#include <QJsonObject>
#include <QGroupBox>

namespace Ui {
class NewCondition;
}

class NewCondition : public QDialog
{
    Q_OBJECT

public:
    explicit NewCondition(QStringList usedConditions, QList<QGroupBox*> groupBoxes, QWidget* parent = nullptr);
    ~NewCondition();

signals:
    void conditionsSelected(QList<QJsonObject> jos, QString joinName) const;

private slots:
    void pushButtonAdd_clicked();
    void pushButtonCancel_clicked();
    void checkBoxSelectAll_toggled(bool checked);
    void checkBoxUnite_toggled(bool checked);
    void radioButtonLeft_clicked();
    void radioButtonRight_clicked();

private:
    Ui::NewCondition* ui;

    QStringList             m_usedConditions;
    QStringList             m_conditionsSelected;
    QMap<QString, QString>  m_allConditions;
    QList<QCheckBox*>       m_allCheckBoxes;
    QList<QGroupBox*>       m_groupBoxes;
    int                     m_childCountLeft;
    int                     m_childCountRight;

    void setup();

    void connectObjects();

    bool canPressAdd() const;
    bool canUnite   () const;
};

#endif // NEWCONDITION_H
