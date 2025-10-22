#ifndef CONDITIONFIELD_H
#define CONDITIONFIELD_H

#include "dragframe.h"

#include <QStringList>
#include <QLineEdit>
#include <QGroupBox>
#include <QJsonObject>
#include <QPair>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>

class ConditionField
{
public:
    ConditionField() = default;

    ConditionField(const QString name) :
        m_title(name)
    {}

    void setTitle     (QString title) { m_title = title; }
    void setConditions(QStringList conditions);
    void setPage      (int page)     { m_page = page; }
    void setPosition  (int position) { m_position = position; }

    void addArgumentSource(QLineEdit* source);

    int  conditionsCount()  const;
    auto condition()        const { return m_condition; }
    auto arguments()        const { return m_arguments; }
    auto argumentsSources() const { return m_argumentsSources; }
    auto title()            const { return m_title; }
    auto page()             const { return m_page; }
    auto position()         const { return m_position; }
    bool isValid()          const { return page() >= 0 && position() >= 0; }

    QStringList  getArgumentsFromConditions(QStringList conditions) const;

private:
    QString             m_title;
    QString             m_condition;
    QStringList         m_arguments;
    QList<QLineEdit*>   m_argumentsSources;
    int                 m_page     {-1};
    int                 m_position {-1};

    void setArguments(QStringList arguments);

    QStringList  extactArgumentsFromString (QString string) const;
};

#endif // CONDITIONFIELD_H
