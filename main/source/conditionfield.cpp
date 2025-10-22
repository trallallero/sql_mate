#include "conditionfield.h"

#include <QLabel>

void ConditionField::setConditions(QStringList conditions)
{
    for(auto& val : conditions)
    {
        m_condition.append(val);
        m_condition.append("\n");
    }
    const auto arguments  = getArgumentsFromConditions(conditions);
    setArguments(arguments);
}

void ConditionField::setArguments(QStringList arguments)
{
    for(auto& val : arguments)
        m_arguments.append(val);
}

void ConditionField::addArgumentSource(QLineEdit* source)
{
    m_argumentsSources.append(source);
}

int ConditionField::conditionsCount() const
{
    return condition().split("\n", QString::SkipEmptyParts).count();
}

QStringList ConditionField::getArgumentsFromConditions(QStringList conditions) const
{
    QStringList arguments;

    for(auto& val : conditions)
    {
        auto args = extactArgumentsFromString(val);
        for(auto& arg : args)
            if(arguments.contains(arg) == false)
                arguments.append(arg);
    }
    return arguments;
}

// QRegularExpression seems not able to extract multiple words (or I don't understand how) so we use here a C++ way
QStringList ConditionField::extactArgumentsFromString(const QString string) const
{
    QStringList arguments;
    QString argument;
    bool started = false;

    for(auto& c : string)
    {
        if(c == '{')
            started = true;
        if (started)
            argument.append(c);
        if (c == '}')
        {
            started = false;
            if (arguments.contains(argument) == false)
                arguments.append(argument);
            argument.clear();
        }
    }
    return arguments;
}
