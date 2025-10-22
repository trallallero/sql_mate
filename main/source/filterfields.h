#ifndef FILTERFIELDS_H
#define FILTERFIELDS_H

#include <QDialog>
#include <QCheckBox>

namespace Ui {
class FilterFields;
}

class FilterFields : public QDialog
{
    Q_OBJECT

public:
    explicit FilterFields(QWidget* parent = nullptr);
    ~FilterFields();

    static QStringList loadProfileSelectedFields();

    static QStringList getSelectedFields() { return m_selectedFields; }
    static QStringList getAllFields     (bool lower);

signals:
    void fieldsSelected(const QList<QString> fields) const;

private slots:
    void checkBoxSelectTenants_toggled(bool checked);
    void pushButtonOk_clicked();
    void pushButtonCancel_clicked();

private:
    Ui::FilterFields* ui;
    QDialog*          m_dialog          { nullptr };
    QList<QCheckBox*> m_fieldsCheckBoxes;

    static QStringList m_selectedFields;
    static QStringList m_allFields;

    void connectObjects();
    void createFields();
    void setSelectedFields();
    void setAllFields();
    static QString getFieldFromLine(QString line);
};

#endif // FILTERFIELDS_H
