#include "conditions.h"
#include "globals.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSizePolicy>
#include <QFrame>
#include <QApplication>
#include <QRegularExpression>
#include <QMessageBox>
#include <QSpacerItem>
#include <QAction>
#include <QLineEdit>
#include <QDebug>

Conditions::Conditions(QWidget* parent)
    : m_parent(parent)
{
    m_conditionFieldHandler = new ConditionFieldHandler(this, m_groupBoxes, m_checkBoxFieldMap, m_frameCheckBoxMap);

    connect(m_conditionFieldHandler, &ConditionFieldHandler::maxNumberOfConditionsReached, this, [this](){
        emit maxNumberOfConditionsReached();
    });
    connect(m_conditionFieldHandler, &ConditionFieldHandler::showMessage, this, [this](QString message){
        emit showMessage(message);
    });
    connect(m_conditionFieldHandler, &ConditionFieldHandler::visibleGroups, this, [this](int groups){
        emit visibleGroups(groups);
    });

    m_contextMenu = new ConditionsContextMenu(parent);

    connect(m_contextMenu, &ConditionsContextMenu::uniteFields , this                   , &Conditions::uniteEnabledFields);
    connect(m_contextMenu, &ConditionsContextMenu::extractField, this                   , &Conditions::extractConditionFromFrame);
    connect(m_contextMenu, &ConditionsContextMenu::renameField , m_conditionFieldHandler, &ConditionFieldHandler::renameField);
}

void Conditions::addConditionFields(QList<QGroupBox*> groupBoxes)
{
    m_groupBoxes = groupBoxes;

    auto jsonConfigObj = Globals::profileConfigurationObject();

    auto fieldsList = m_conditionFieldHandler->getOrderedFields();

    for(int groupsCount = 0; groupsCount < fieldsList.count(); ++groupsCount)
    {
        for(int i = 0; i < fieldsList[groupsCount].count(); ++i)
        {
            if(i > Globals::maxNumberOfConditionsPerGroup())
            {
                emit maxNumberOfConditionsReached();
                break;
            }
            auto gb = groupBoxes[groupsCount];
            const auto jo = fieldsList[groupsCount].at(i);
            createConditionField(gb, jo, jsonConfigObj);
        }
    }
}

void Conditions::addConditionField(QList<QGroupBox*> groupBoxes, QJsonObject jo)
{
    fieldUnselected();

    QJsonObject joValues;
    QJsonObject joConfig;

    joValues["title"     ] = jo["title"].toString();
    joValues["conditions"] = jo["conditions"].toArray();
    joValues["page"      ] = jo["page"].toInt();
    joValues["position"  ] = jo["position"].toInt();

    QJsonObject joTemp1, joTemp2;
    joTemp1["cbNot"   ] = jo["not"  ].toBool();
    joTemp1["cbNull"  ] = jo["null" ].toBool();
    joTemp1["cbIn"    ] = jo["in"   ].toBool();
    joTemp1["cbLike"  ] = jo["like" ].toBool();
    joTemp1["cbCase"  ] = jo["case" ].toBool();

    joTemp1["lineEdit_" + jo["title"].toString()] = jo["value"].toString();

    joTemp1["checked" ] = true;

    auto title = jo["title"].toString();
    auto conditionName = "groupBoxCondition_" + title;

    joTemp2[conditionName] = joTemp1;

    joConfig["conditions"] = joTemp2;

    auto page = jo["page"].toInt();
    auto gb = groupBoxes[page];

    auto w = createConditionField(gb, joValues, joConfig);
    if (w)
        w->setFocus();
}

void Conditions::addConditionField(QJsonObject jo)
{
    addConditionField(m_groupBoxes, jo);
}

// TODO: move this to separate library (maybe depending on the db allthough it's ansi sql)
QString Conditions::getConditions() const
{
    QString conditions;

    for(auto& item : m_frameCheckBoxMap.toStdMap())
    {
        if (item.second->isChecked() == false)
            continue; // condition is uncheked

        auto field            = m_checkBoxFieldMap[item.second];
        auto condition        = field.condition().trimmed();
        auto arguments        = field.arguments();
        auto argumentsSources = field.argumentsSources();
        auto cBoxes           = item.first->findChildren<QCheckBox*>();
        auto isNot            = false;
        auto isNull           = false;
        auto isIn             = false;
        auto isLike           = false;
        auto isCase           = false;
        auto trimmedValue     = argumentsSources[0]->text().trimmed();

        for (auto& cb : cBoxes)
        {
            if(cb->objectName() == "cbIn" && cb->isChecked())
                isIn = true;
            else if(cb->objectName() == "cbNull" && cb->isChecked())
                isNull = true;
            else if(cb->objectName() == "cbNot" && cb->isChecked())
                isNot = true;
            else if(cb->objectName() == "cbLike" && cb->isChecked())
                isLike = true;
            else if(cb->objectName() == "cbCase" && cb->isChecked())
                isCase = true;
        }
        // NOTE: if there are checkboxes means there can be only one argument (arguments[0] and argumentsSources[0])
        if (isIn == true) // isIn and isLike are exclusive so don't need to check isLike here
        {
            if (isNot == false)
                condition.replace(" = ", " IN ");
            else
                condition.replace(" = ", " NOT IN ");

            QString valuesString = trimmedValue.replace("\n", " ").replace(" ", "', '"); // TODO: what if is not string?
            qDebug() << valuesString;

            if(condition.contains("'"))
                condition.replace("'" + arguments[0] + "'", "('" + valuesString + "')");
            else
                condition.replace(arguments[0], "(" + valuesString + ")");
            if (isCase)
                condition = convertConditionToLower(condition);
            condition.append("\n");
        }
        else if (isIn == false && isNot == true && isLike == false)
        {
            if (trimmedValue.isEmpty())
            {
                if(isNull == false)
                {
                    condition.replace(" = ", " <> ").replace(arguments[0], trimmedValue);
                    condition.append("\n");
                }
                else
                {
                    if(condition.contains("'"))
                        condition.replace("=", " IS NOT").replace("'" + arguments[0] + "'", "NULL");
                    else
                        condition.replace("=", " IS NOT").replace(arguments[0], "NULL");
                }
            }
            else
            {
                condition.replace(" = ", " <> ");
                condition.replace(arguments[0], trimmedValue);
            }
            if (isCase)
                condition = convertConditionToLower(condition);
            condition.append("\n");
        }
        else if (isLike == true) // isIn and isLike are exclusive so don't need to check isIn here
        {
            if (isNot == false)
                condition.replace(" = ", " LIKE ");
            else
                condition.replace(" = ", " NOT LIKE ");

            QString valuesString = trimmedValue.replace("\n", " ");
            condition.replace(arguments[0], "%" + valuesString + "%");
            if (isCase)
                condition = convertConditionToLower(condition);
            condition.append("\n");
        }
        else
        {   // Here the only place where there might be more than 1 argument so we cycle through them
            auto splittedConditions = condition.split("\n", QString::SkipEmptyParts);
            qDebug() << splittedConditions;
            condition.clear();

            for(int i = 0; i < arguments.size(); ++i)
            {
                auto value = argumentsSources[i]->text().trimmed();

                auto arg = arguments[i];
                auto singleCondition = *std::find_if(splittedConditions.begin(), splittedConditions.end(), [arg](QString s){
                    return s.contains(arg);
                });
                if (value.isEmpty()) // if empty we add a NULL check
                {
                    auto singleConditionCopy = singleCondition;
                    singleCondition.replace("AND ", "AND (").replace(arguments[i], value);
                    singleConditionCopy.replace("AND ", " OR ").replace(QString("= '%1'").arg(arguments[i]), " IS NULL");
                    singleConditionCopy.append(")\n");
                    singleCondition.append(singleConditionCopy);
                }
                else
                {
                    singleCondition.replace(arguments[i], argumentsSources[i]->text().trimmed());
                    singleCondition.append("\n");
                }
                if (isCase)
                    singleCondition = convertConditionToLower(singleCondition);
                condition.append(singleCondition);
            }
        }
        conditions.append(condition);
    }
    return conditions.trimmed();
}

QStringList Conditions::getConditionNames(bool lower, bool onlyOneArgument) const
{
    QStringList conditionsNames;

    for(auto& item : m_checkBoxFieldMap.toStdMap())
    {
        auto field  = item.second;
        auto argTot = field.arguments().size();
        if (onlyOneArgument && argTot > 1)
            continue;

        if (lower)
            conditionsNames.append(field.title().toLower());
        else
            conditionsNames.append(field.title());
    }
    return conditionsNames;
}

QList<QJsonObject> Conditions::getJsonConditions() const
{
    QList<QJsonObject> jsonObjList;

    for(auto& item : m_checkBoxFieldMap.toStdMap())
    {
        auto joField = QJsonObject{{"title", item.second.title()}};
        auto conditions = item.second.condition().trimmed().split("\n");
        joField["conditions"] = QJsonArray::fromStringList(conditions);
        joField["page"      ] = item.second.page();
        joField["position"  ] = item.second.position();
        jsonObjList.append(joField);
    }
    return jsonObjList;
}

QVariantMap Conditions::getJsonConditionsValues() const
{
    QMap<QString, QJsonObject> jsonObjMap;

    for(auto& item : m_frameCheckBoxMap.toStdMap())
    {
        QJsonObject jsonObj;
        jsonObj["checked"] = item.second->isChecked();
        auto children = item.first->children();
        for (auto& child : children)
        {
            auto name = child->objectName();
            if (name.startsWith("groupBoxOptionsCondition_"))
            {
                for (auto& child2 : child->children())
                {
                    auto name2 = child2->objectName();
                    if (name2.isEmpty() == false)
                    {
                        auto cb = dynamic_cast<QCheckBox*>(child2);
                        if (cb)
                            jsonObj[name2] = cb->isChecked();
                    }
                }
            }
            else if (name.startsWith("lineEdit_"))
            {
                auto le = dynamic_cast<QLineEdit*>(child);
                if (le)
                    jsonObj[name] = le->text();
            }
        }
        jsonObjMap[item.first->objectName()] = jsonObj;
    }

    QVariantMap variantConditionMap;
    QMapIterator<QString, QJsonObject> i(jsonObjMap);
    while (i.hasNext())
    {
        i.next();
        variantConditionMap.insert(i.key(), i.value());
    }
    return variantConditionMap;
}

void Conditions::removeConditionFields()
{
    fieldUnselected();
    for(auto& item : m_frameCheckBoxMap.toStdMap())
        delete item.first;
    m_checkBoxFieldMap.clear();
    m_frameCheckBoxMap.clear();
    m_currentSelectedFrame   = nullptr;
    m_currentDraggingFrame   = nullptr;
    m_currentDropFrame       = nullptr;

    for(auto& gb : m_groupBoxes)
    {
        auto childs = gb->findChildren<QFrame*>(QRegularExpression("separatorCondition_.*"), Qt::FindDirectChildrenOnly);
        for(auto& child : childs)
        {
            gb->layout()->removeWidget(child);
            delete child;
        }
    }
    m_conditionFieldHandler->updateLayoutSize();
}

void Conditions::clearAll()
{
    for(auto& item : m_frameCheckBoxMap.toStdMap())
    {
        auto children = item.first->children();
        for (auto& child : children)
        {
            auto name = child->objectName();
            if (name.startsWith("groupBoxOptionsCondition_"))
            {
                for (auto& child2 : child->children())
                {
                    auto name2 = child2->objectName();
                    if (name2.isEmpty() == false)
                    {
                        auto cb = dynamic_cast<QCheckBox*>(child2);
                        if (cb)
                            cb->setChecked(false);
                    }
                }
            }
            else if (name.startsWith("lineEdit_"))
            {
                auto le = dynamic_cast<QLineEdit*>(child);
                if (le)
                {
                    le->setText({});
                }
            }
        }
    }
    fieldUnselected();
}

void Conditions::deactivateAll()
{
    for(auto& p : m_checkBoxFieldMap.toStdMap())
        p.first->setChecked(false);
    fieldUnselected();
}

bool Conditions::moveFrameTo(QFrame* frame, QGroupBox* groupBox, int page)
{
    auto field = m_conditionFieldHandler->getFieldByFrame(frame);
    if(field.page() == page)
        return false;

    frame->setParent(groupBox);

    auto gb = qobject_cast<QGroupBox*>(frame->parent());
    gb->layout()->removeWidget(frame);

    auto vlo = qobject_cast<QVBoxLayout*>(groupBox->layout());
    if(vlo)
    {
        auto position = vlo->count() - 1;
        vlo->insertWidget(position, frame);
        m_conditionFieldHandler->setFieldCoordinates(frame, page, position);
    }

    m_conditionFieldHandler->updateFieldsPositionAfterRemovingField(page == 0 ? 1 : 0);
    m_conditionFieldHandler->updateLayoutSize();
    return true;
}

QWidget* Conditions::createConditionField(
        QGroupBox*         parentGb,
        const QJsonObject& joValues,
        const QJsonObject& joConfig,
        QString            value,
        bool               enabled)
{
    auto cfd = ConditionFieldHandler::createConditionField(parentGb, joValues, joConfig, value, enabled);
    if(cfd.widget == nullptr)
        return nullptr;

    m_checkBoxFieldMap[cfd.cb]    = cfd.field;
    m_frameCheckBoxMap[cfd.frame] = cfd.cb;

    connectFieldObjects(cfd);

    m_conditionFieldHandler->insertWidgetInLayout(parentGb->layout(), cfd.frame);

    onCheckBoxTitleToggled(cfd.frame, cfd.enabled);

    return cfd.widget;
}

void Conditions::connectFieldObjects(ConditionFieldData& cfd)
{
    connect(cfd.frame, &DragFrame::dragStarted     , this, &Conditions::dragStarted       );
    connect(cfd.frame, &DragFrame::frameDropped    , this, &Conditions::frameDropped      );
    connect(cfd.frame, &DragFrame::dragEnteredFrame, this, &Conditions::setDropTargetFrame);
    connect(cfd.frame, &DragFrame::showContextMenu , this, &Conditions::showContextMenu   );
    connect(cfd.frame, &DragFrame::fieldSelected   , this, &Conditions::fieldSelected     );
    connect(cfd.frame, &DragFrame::fieldUnselected , this, &Conditions::fieldUnselected   );

    if (cfd.cb)
    {
        connect(cfd.cb, &QCheckBox::destroyed, this, [this, cfd](){
            auto child = cfd.frame->findChild<QLineEdit*>("lineEdit_" + cfd.field.title());
            emit checkboxDeleted(child);
        });

        connect(cfd.cb, &QCheckBox::toggled, this, [this, cfd](bool toggled){
            fieldUnselected();
            onCheckBoxTitleToggled(cfd.frame, toggled);
        });
    }

    for (auto& name : ConditionFieldHandler::checkBoxesConditionNames())
    {
        if (cfd.cbs[name])
            connect(cfd.cbs[name], &QCheckBox::toggled, this, [this, cfd, name](bool toggled){
                fieldUnselected();
                emit lineEditFocusRequest();
                if (name == "cbIn" && toggled)
                {
                    if(cfd.cbs["cbLike"])
                        cfd.cbs["cbLike"]->setChecked(false);
                    if(cfd.cbs["cbNull"])
                        cfd.cbs["cbNull"]->setChecked(false);
                }
                else if (name == "cbLike" && toggled)
                {
                    if(cfd.cbs["cbIn"])
                        cfd.cbs["cbIn"]->setChecked(false);
                    if(cfd.cbs["cbNull"])
                        cfd.cbs["cbNull"]->setChecked(false);
                }
                else if (name == "cbNull" && toggled)
                {
                    if(cfd.cbs["cbIn"])
                        cfd.cbs["cbIn"]->setChecked(false);
                    if(cfd.cbs["cbLike"])
                        cfd.cbs["cbLike"]->setChecked(false);
                }
            });
    }

    if (cfd.sle)
        connect(cfd.sle, &QLineEdit::textChanged, this, [cfd](const QString& text){
            if(cfd.cbs["cbIn"])
            {
                auto hasWords = text.split(QRegularExpression("[\\s\n\r]"), QString::SkipEmptyParts).count() > 1;
                cfd.cbs["cbIn"]->setEnabled(hasWords);
                cfd.cbs["cbIn"]->setChecked(hasWords);
            }
            if(cfd.cbs["cbNull"])
            {
                if(text.trimmed().isEmpty() == false)
                {
                    cfd.cbs["cbNull"]->setChecked(false);
                    cfd.cbs["cbNull"]->setEnabled(false);
                }
                else
                {
                    auto checked = cfd.frame->findChild<QCheckBox*>("checkBoxTitle")->isChecked();
                    cfd.cbs["cbNull"]->setEnabled(checked);
                }
            }
        });

    if (cfd.bt)
        connect(cfd.bt, &QPushButton::clicked, this, [this, cfd](){
            fieldUnselected();
            auto result = QMessageBox::question(m_parent, tr("ATTENZIONE"), tr("Sicuro di voler eliminare la condizione?"),
                                                QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::Cancel);
            if (result == QMessageBox::StandardButton::Yes)
                deleteField(cfd.field.title(), cfd.frame);
        });

    if(cfd.sle == nullptr)
    {
        for (auto& le : cfd.les)
        {
            le->installEventFilter(this);
            connect(le, &QLineEdit::returnPressed, this, &Conditions::lineEditEnterPressed);
        }
    }
    else
    {
        cfd.sle->installEventFilter(this);
        connect(cfd.sle, &QLineEdit::returnPressed, this, &Conditions::lineEditEnterPressed);
    }
}

void Conditions::onCheckBoxTitleToggled(DragFrame* frame, bool toggled)
{
    bool hasWords = false;
    bool hasText  = false;
    auto children1 = frame->findChildren<QLineEdit*>();
    for(auto& child : children1)
    {
        hasWords = child->text().split(QRegularExpression("[\\s\n\r]"), QString::SkipEmptyParts).count() > 1;
        hasText  = child->text().trimmed().isEmpty() == false;
        child->setEnabled(toggled);
    }

    auto children2 = frame->findChildren<QCheckBox*>();
    for(auto& child : children2)
    {
        if (child->objectName() != "checkBoxTitle")
        {
            if (child->objectName() == "cbIn")
            {
                if (hasWords == false)
                    child->setChecked(false);
                child->setEnabled(toggled && hasWords);
            }
            else if(child->objectName() == "cbNull")
            {
                if(hasText || toggled == false)
                {
                    child->setChecked(false);
                    child->setEnabled(false);
                }
                else if(hasText == false && toggled)
                {
                    child->setEnabled(true);
                }
            }
            else
            {
                child->setEnabled(toggled);
            }
        }
    }

    if (toggled)
    {
        if (children1.count() > 0)
            children1[0]->setFocus();
        frame->setProperty("conditionFrame", "enabled");
    }
    else
    {
        frame->setProperty("conditionFrame", "disabled");
        emit lineEditFocusRequest();
    }
}

void Conditions::deleteField(QString name, QFrame* parentFrame)
{
    m_conditionFieldHandler->deleteField(name, parentFrame);
    m_currentSelectedFrame = nullptr;
}

void Conditions::uniteSelectedFields(QStringList names, QString joinName)
{
    UniteConditionsType predicate = [this](std::pair<QFrame* const, QCheckBox*>& item, QStringList names){
        auto name = m_conditionFieldHandler->getFieldByFrame(item.first).title();
        return names.contains(name);
    };

    uniteConditions(names, joinName, predicate);
}

void Conditions::uniteEnabledFields(DragFrame*, QString title)
{
    UniteConditionsType predicate = [](std::pair<QFrame* const, QCheckBox*>& item, QStringList){
        return item.second->isChecked();
    };

    uniteConditions({}, title, predicate);
}

void Conditions::uniteConditions(QStringList names, QString joinName, UniteConditionsType predicate)
{
    int index = 0;
    int page = 0;
    QStringList conditions;
    QMap<QString, QFrame*> fieldsToDelete;
    for(auto& item : m_frameCheckBoxMap.toStdMap())
    {
        if(predicate(item, names))
        {
            auto field = m_conditionFieldHandler->getFieldByFrame(item.first);
            if(index == 0)
                page = field.page();
            conditions.append(field.condition());
            fieldsToDelete[field.title()] = item.first;
            ++index;
        }
    }
    for(auto& field : fieldsToDelete.toStdMap())
        deleteField(field.first, field.second);

    QJsonObject jo;
    jo["conditions"] = QJsonArray::fromStringList(conditions);
    jo["title"     ] = joinName;
    jo["page"      ] = page;
    jo["position"  ] = m_conditionFieldHandler->getFieldsCountPerPage(page);

    addConditionField(jo);
    m_parent->repaint(); // the shadow of deleted enabled frames seems to not be removed so force a repaint
}

void Conditions::extractConditionFromFrame(DragFrame* frame, QString conditionTitle)
{
    if(conditionTitle.isEmpty())
    {
        auto les  = frame->findChildren<QLineEdit*>(QRegularExpression("lineEdit_.*"));
        for(auto& le : les)
        {
            auto condTitles = le->objectName().split("lineEdit_", QString::SkipEmptyParts);
            if(condTitles.count() == 1)
                extractConditionFromFrame(frame, condTitles[0]); // recursion here
        }
        return;
    }
    auto le  = frame->findChild<QLineEdit*>("lineEdit_" + conditionTitle);
    auto lbl = frame->findChild<QLabel*>("conditionLabel_" + conditionTitle);
    if(le && lbl)
    {
        auto field   = m_conditionFieldHandler->getFieldByFrame(frame);
        auto value   = le->text().trimmed();
        auto enabled = le->isEnabled();

        frame->layout()->removeWidget(le);
        delete le;
        frame->layout()->removeWidget(lbl);
        delete lbl;

        auto conditions = field.condition().split("\n", QString::SkipEmptyParts);
        auto it = *std::find_if(conditions.begin(), conditions.end(), [conditionTitle](const QString& str){
            return str.contains(conditionTitle);
        });
        conditions.removeOne(it);
        field.setConditions(conditions);
        m_conditionFieldHandler->setFieldByFrame(frame, field);

        QJsonObject joValues {
            {"title"     , conditionTitle    },
            {"conditions", QJsonArray({it}) },
            {"page"      , field.page()      },
            {"position"  , field.position()  }
        };

        QJsonObject joConfig {
            { "conditions", QJsonObject {
                { "groupBoxCondition_" + conditionTitle, QJsonObject {
                    { "lineEdit_" + conditionTitle, value},
                    { "checked", enabled}
            }}}}
        };

        auto w = createConditionField(m_groupBoxes[field.page()], joValues, joConfig);
        if(w)
            w->setFocus();
        if(frame->findChildren<QLineEdit*>(QRegularExpression("lineEdit_.*")).count() <= 0)
        {
            auto cb = m_frameCheckBoxMap[frame];
            if(cb)
                deleteField(cb->text(), frame);
        }
    }
}

QString Conditions::convertConditionToLower(QString condition) const
{
    QRegularExpression re(".*\\w+[ ]+(\\w+\\.[\"]?\\w+[\"]?)[ ]+([^\\s]+)(.*)");
    QRegularExpressionMatch match = re.match(condition);
    if (match.hasMatch())
    {
        auto key   = match.captured(1).trimmed();
        auto value = match.captured(3).trimmed();
        return condition.replace(key, QString("LOWER(%1)").arg(key)).replace(value, value.toLower());
    }
    return condition;
}

void Conditions::moveCurrentField(FieldDirection fd)
{
    if(m_currentSelectedFrame == nullptr)
        return;

    auto field1   = m_conditionFieldHandler->getFieldByFrame(m_currentSelectedFrame);
    auto position = field1.position();
    auto page     = field1.page();
    auto tot      = m_conditionFieldHandler->fieldsPerPage(page);

    if((fd == FD_UP && position == 0) || (fd == FD_DOWN && position >= tot - 1))
        return;

    auto newPosition = fd == FD_UP ? position - 1 : position + 1;
    field1.setPosition(newPosition);

    auto fieldIter2 = std::find_if(m_checkBoxFieldMap.keyBegin(), m_checkBoxFieldMap.keyEnd(), [this, position, page, fd](auto& item){
        if(fd == FD_UP)
            return m_checkBoxFieldMap[item].page() == page && m_checkBoxFieldMap[item].position() == position - 1;
        else
            return m_checkBoxFieldMap[item].page() == page && m_checkBoxFieldMap[item].position() == position + 1;
    });

    if(fieldIter2 == m_checkBoxFieldMap.keyEnd())
        return;
    auto field2 = m_checkBoxFieldMap[*fieldIter2];
    field2.setPosition(position);

    m_conditionFieldHandler->setFieldByFrame(m_currentSelectedFrame, field1);
    m_checkBoxFieldMap[*fieldIter2] = field2;

    m_conditionFieldHandler->updateFieldsPositionAfterMovingField(page);
    m_currentSelectedFrame->frameSelected();
    emit frameSelected(m_currentSelectedFrame, newPosition, m_conditionFieldHandler->getFieldsCountPerPage(page));
}

void Conditions::setDropTargetFrame(QFrame* frame)
{
    if(m_currentDropFrame && m_currentDropFrame->objectName() == frame->objectName())
        return;

    m_currentDropFrame = frame;
}

//Shortcuts in line edit to trigger: ALT+I for In, ALT+L for Like, ALT+N for Not
bool Conditions::eventFilter(QObject* obj, QEvent* event)
{
    auto le = qobject_cast<QLineEdit*>(obj);

    if (le && event->type() == QEvent::MouseButtonPress)
    {
        fieldUnselected();
    }
    else if (le && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent *>(event);
        const auto modifiers = key->modifiers();
        if(modifiers & Qt::AltModifier && !(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier))
        {
            const auto keyPressed = key->key();
            if(keyPressed == Qt::Key_I || keyPressed == Qt::Key_L || keyPressed == Qt::Key_N)
            {
                auto children = le->parent()->findChildren<QCheckBox*>();
                if(children.count() > 0)
                {
                    for(auto& child : children)
                    {
                        if(key->key() == Qt::Key_I && child->objectName() == "cbIn")
                        {
                            auto cb = qobject_cast<QCheckBox*>(child);
                            cb->toggle();
                            return true;
                        }
                        else if (key->key() == Qt::Key_L && child->objectName() == "cbLike")
                        {
                            auto cb = qobject_cast<QCheckBox*>(child);
                            cb->toggle();
                            return true;
                        }
                        else if (key->key() == Qt::Key_N && child->objectName() == "cbNot")
                        {
                            auto cb = qobject_cast<QCheckBox*>(child);
                            cb->toggle();
                            return true;
                        }
                    }
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void Conditions::showContextMenu(DragFrame* frame)
{
    m_contextMenu->showContextMenu(frame, m_conditionFieldHandler->getFieldByFrame(frame), m_checkBoxFieldMap);
}

void Conditions::fieldSelected(DragFrame* frame)
{
    m_currentSelectedFrame = frame;
    frame->setProperty("conditionFrame", "selected");
    auto field = m_conditionFieldHandler->getFieldByFrame(frame);
    auto total = m_conditionFieldHandler->getFieldsCountPerPage(field.page());
    emit frameSelected(frame, field.position(), total);
}

void Conditions::fieldUnselected()
{
    if(m_currentSelectedFrame)
    {
        auto enabled = m_frameCheckBoxMap[m_currentSelectedFrame]->isChecked();
        m_currentSelectedFrame->setProperty("conditionFrame", enabled ? "enabled" : "disabled");
        m_currentSelectedFrame = nullptr;
        emit frameSelected(nullptr, -1, -1);
    }
}

void Conditions::dragStarted(QFrame* frame)
{
    m_currentDraggingFrame = frame;
    emit dragStartedSignal(frame);
}

void Conditions::frameDropped(QFrame*)
{
    if(m_currentDropFrame && m_currentDraggingFrame && m_currentDraggingFrame != m_currentDropFrame)
        m_conditionFieldHandler->swapFields(m_currentDraggingFrame, m_currentDropFrame);

    m_currentDropFrame     = nullptr;
    m_currentDraggingFrame = nullptr;
}
