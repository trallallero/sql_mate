#include "conditionfieldhandler.h"
#include "globals.h"

#include <QLabel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

QVector<QPair<QString, QString>> ConditionFieldHandler::checkBoxesConditions {
    {"NOT" , "cbNot"},
    {"NULL", "cbNull"},
    {"IN"  , "cbIn"},
    {"LIKE", "cbLike"},
    {"CASE", "cbCase"}
};

ConditionFieldHandler::ConditionFieldHandler(
        QObject*           parent,
        QList<QGroupBox*>& groupBoxes,
        CheckBoxFieldMap&  checkBoxFieldMap,
        FrameCheckBoxMap&  frameCheckBoxMap)
    : QObject(parent)
    , m_groupBoxes(groupBoxes)
    , m_checkBoxFieldMap(checkBoxFieldMap)
    , m_frameCheckBoxMap(frameCheckBoxMap)
{
}

ConditionField ConditionFieldHandler::getFieldByFrame(QFrame* frame) const
{
    return m_checkBoxFieldMap[m_frameCheckBoxMap[frame]];
}

void ConditionFieldHandler::setFieldByFrame(QFrame* frame, ConditionField field)
{
    m_checkBoxFieldMap[m_frameCheckBoxMap[frame]] = field;
}

void ConditionFieldHandler::swapFields(QFrame* frame1, QFrame* frame2)
{
    auto field1 = getFieldByFrame(frame1);
    auto field2 = getFieldByFrame(frame2);

    if(field1.isValid() && field2.isValid())
    {
        auto position = field1.position();
        auto page     = field1.page();
        field1.setPosition(field2.position());
        field2.setPosition(position);
        field1.setPage    (field2.page());
        field2.setPage    (page);

        setFieldByFrame(frame1, field1);
        setFieldByFrame(frame2, field2);
        updateFieldsPositionAfterMovingField(field1.page());
        if(field2.page() != field1.page())
            updateFieldsPositionAfterMovingField(field2.page());
        qobject_cast<DragFrame*>(frame1)->restoreOriginalStyleSheet();
        qobject_cast<DragFrame*>(frame2)->restoreOriginalStyleSheet();
    }
}

void ConditionFieldHandler::setFieldCoordinates(QFrame* frame, int page, int position)
{
    auto field = getFieldByFrame(frame);
    field.setPage(page);
    field.setPosition(position);
    setFieldByFrame(frame, field);
}

OrderedFields ConditionFieldHandler::getOrderedFields() const
{
    auto result = getInitializedOrderedFields();

    QFile inputFileConditions(Globals::currentSchema().isEmpty()
            ? Globals::filenameProfileConditionFields()
            : Globals::filenameSchemaConditionFields());

    inputFileConditions.open(QIODevice::ReadOnly | QIODevice::Text);
    const auto jsonValues = inputFileConditions.readAll();
    inputFileConditions.close();

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonValues);
    const QJsonArray    ja      = jsonDoc.array();

    for(int i = 0; i < ja.size(); ++i)
    {
        const auto jo = ja.at(i).toObject();
        auto position = jo["position"].toInt();
        auto page     = jo["page"    ].toInt();
        if((page == 0 || page == 1) && position >= 0 && position < Globals::maxNumberOfConditionsPerGroup() )
            result[page][position] = jo;
        else
            emit maxNumberOfConditionsReached();
    }
    return result;
}

OrderedFields ConditionFieldHandler::getInitializedOrderedFields() const
{
    OrderedFields result;
    for(int i = 0; i < Globals::numberOfGroups(); ++i)
    {
        result.append(QList<QJsonObject>());
        for(int n = 0; n < Globals::maxNumberOfConditionsPerGroup(); ++n)
            result[i].append(QJsonObject());
    }
    return result;
}

int ConditionFieldHandler::getFieldsCountPerPage(int page) const
{
    return m_groupBoxes[page]->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();
}

void ConditionFieldHandler::renameField(DragFrame* frame, QString title)
{
    auto field = getFieldByFrame(frame);
    auto map   = m_checkBoxFieldMap.toStdMap();
    auto exist = std::count_if(map.begin(), map.end(), [title](const std::pair<QCheckBox*, ConditionField>& p){
            return p.second.title() == title;
    });
    if(exist > 0)
    {
        if(field.title() != title)
            emit showMessage(tr("Nome già in uso. Usare un titolo univoco!"));
        return;
    }
    field.setTitle(title);
    setFieldByFrame(frame, field);
    m_frameCheckBoxMap[frame]->setText(title);
}

void ConditionFieldHandler::updateLayoutSize()
{
    int index = 0;
    int visibleGroupsCount = 0;
    for(auto& gb : m_groupBoxes)
    {
        auto count = getFieldsCountPerPage(index++);
        gb->setSizePolicy(count > 0 ? QSizePolicy::Minimum : QSizePolicy::Maximum, QSizePolicy::Minimum);
        if(count > 0)
            ++visibleGroupsCount;
    }
    emit visibleGroups(visibleGroupsCount);
}

void ConditionFieldHandler::updateFieldsPositionAfterRemovingField(int page)
{
    int position = 0;
    for(auto& item : m_checkBoxFieldMap)
        if(item.page() == page)
            item.setPosition(position++);
}

void ConditionFieldHandler::updateFieldsPositionAfterMovingField(int page)
{
    auto gb = m_groupBoxes[page];

    for(auto& item : m_frameCheckBoxMap.toStdMap())
    {
        if(item.first->parentWidget() == gb)
            gb->layout()->removeWidget(item.first);
    }

    QMap<int, QFrame*> items;
    auto predicate = [this, page](auto& item){
        return m_checkBoxFieldMap[item].page() == page;
    };
    for (auto it = m_checkBoxFieldMap.keyBegin();
         (it = std::find_if(it, m_checkBoxFieldMap.keyEnd(), predicate)) != m_checkBoxFieldMap.keyEnd();
         ++it)
        items[m_checkBoxFieldMap[*it].position() + 1] = qobject_cast<QFrame*>((*it)->parent()->parent());

    for(auto& item : items)
        insertWidgetInLayout(gb->layout(), item);

    updateLayoutSize();
}

void ConditionFieldHandler::insertWidgetInLayout(QLayout* lo, QWidget* w)
{
    auto vlo = qobject_cast<QVBoxLayout*>(lo);
    if(vlo)
        vlo->insertWidget(vlo->count() - 1, w);
    updateLayoutSize();
}

void ConditionFieldHandler::deleteField(QString name, QFrame* parentFrame)
{
    int page = -1;
    for (auto it = m_checkBoxFieldMap.begin(); it != m_checkBoxFieldMap.end(); ++it)
    {
        if (it.key()->text() == name)
        {
            page     = it.value().page();
            m_checkBoxFieldMap.erase(it);
            break;
        }
    }
    m_frameCheckBoxMap.take(parentFrame);

    delete parentFrame;
    if(page >= 0)
        updateFieldsPositionAfterRemovingField(page);
    updateLayoutSize();
}

int ConditionFieldHandler::fieldsPerPage(int page) const
{
    return std::count_if(m_checkBoxFieldMap.cbegin(), m_checkBoxFieldMap.cend(), [page](auto& item){ return item.page() == page; });
}

QStringList ConditionFieldHandler::checkBoxesConditionNames()
{
    QStringList names;
    for (auto& item : checkBoxesConditions)
        names.append(item.second);
    return names;
}

/************************ STATIC METHODS *****************************/

ConditionFieldData ConditionFieldHandler::createConditionField(
        QGroupBox* parentGb,
        const QJsonObject& joValues,
        const QJsonObject& joConfig,
        QString value,
        bool enabled)
{
    QWidget* returnWidget { nullptr };

    const auto name = joValues.value("title").toString();
    if(name.isEmpty())
        return {};
    const auto conditions = joValues.value("conditions").toVariant().toStringList();

    ConditionField field(name);
    field.setConditions(conditions);
    field.setPage      (joValues.value("page").toInt());
    field.setPosition  (joValues.value("position").toInt());

    const auto arguments = field.getArgumentsFromConditions(conditions);

    ConditionFieldData cfd;

    auto o_name = "groupBoxCondition_" + name;
    auto values  = createFieldFrame(cfd, parentGb, o_name, name);
    auto frame   = values.first;
    auto vLayout = values.second;

    QList<QLineEdit*> les;

    for(auto& arg : arguments)
    {
        auto lEdit = createFieldLineEdit(frame, value, o_name, joConfig, arg);
        les.append(lEdit);

        if (arg == arguments[0]) // return the first QLineEdit created
            returnWidget = lEdit;

        if (arguments.size() > 1)
        {
            auto argCopy = arg; // don't replace the original
            argCopy.replace("{", "").replace("}", "");
            auto label = new QLabel(argCopy, frame);
            label->setObjectName("conditionLabel_" + argCopy);
            vLayout->addWidget(label);
        }
        else // if only 1 argument, we don't need a label for each argument so skip it
        {
            auto gbItems = createFieldCheckBoxes(cfd, frame, name, o_name, joConfig);
            vLayout->addWidget(gbItems);
            cfd.sle = lEdit;
        }

        vLayout->addWidget(lEdit);
        field.addArgumentSource(lEdit);
    }

    auto cb = frame->findChild<QCheckBox*>("checkBoxTitle");

    cfd.widget  = returnWidget;
    cfd.frame   = frame;
    cfd.cb      = cb;
    cfd.les     = les;
    cfd.field   = field;
    cfd.enabled = setFieldEnabled(cb, o_name, joConfig, enabled);
    return cfd;
}

QPair<DragFrame*, QVBoxLayout*> ConditionFieldHandler::createFieldFrame(
        ConditionFieldData& cfd,
        QGroupBox* parentGb,
        QString objName,
        QString fieldName)
{
    auto frame = new DragFrame(parentGb);
    frame->setObjectName(objName);

    auto cbGroupBox = new QGroupBox(frame);
    cbGroupBox->setObjectName("groupBoxConditionTitle");
    auto hCbLayout = new QHBoxLayout;
    hCbLayout->setContentsMargins(0, 0, 0, 0);
    cbGroupBox->setLayout(hCbLayout);

    auto cb = new QCheckBox(fieldName, frame);
    cb->setObjectName("checkBoxTitle");
    cb->setCursor(Qt::PointingHandCursor);
    cb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    hCbLayout->addWidget(cb);
    auto button = createDeleteConditionButton();
    cfd.bt = button;
    hCbLayout->addWidget(button);

    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(5, 5, 5, 5);
    vLayout->setSpacing(3);
    frame->setLayout(vLayout);
    vLayout->addWidget(cbGroupBox);

    return {frame, vLayout};
}

QLineEdit* ConditionFieldHandler::createFieldLineEdit(
        QFrame* frame,
        QString userText,
        QString objName,
        const QJsonObject& joConfig,
        QString fieldName)
{
    fieldName = fieldName.replace("{", "").replace("}", "");

    auto lEdit = new QLineEdit(frame);
    lEdit->setCursor(Qt::ArrowCursor);
    lEdit->setObjectName("lineEdit_" + fieldName);
    auto text = userText.isEmpty()? joConfig["conditions"][objName][lEdit->objectName()].toString("") : userText;
    lEdit->setText(text);
    return lEdit;
}

QGroupBox* ConditionFieldHandler::createFieldCheckBoxes(
        ConditionFieldData& cfd,
        QFrame* frame,
        QString name,
        QString objName,
        const QJsonObject& joConfig)
{
    // only conditions with 1 input can be "IN" and "NOT" and "LIKE"
    auto gbItems = new QGroupBox(frame);
    gbItems->setCheckable(false);
    gbItems->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    gbItems->setObjectName("groupBoxOptionsCondition_" + name);

    QFont font("Arial", 6);

    auto hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(10, 0, 3, 0);
    gbItems->setLayout(hLayout);

    for (auto& item : checkBoxesConditions)
    {
        auto* cb = new QCheckBox(item.first, gbItems);
        cb->setObjectName(item.second);
        cb->setCursor(Qt::PointingHandCursor);
        cb->setFont(font);
        hLayout->addWidget(cb);
        auto checked = joConfig["conditions"][objName][item.second].toBool(false);
        cb->setChecked(checked);
        cfd.cbs[item.second] = cb;
    }
    return gbItems;
}

bool ConditionFieldHandler::setFieldEnabled(
        QCheckBox* cb,
        QString objName,
        const QJsonObject& joConfig,
        bool enabled)
{
    if (enabled)
    {
        cb->setChecked(enabled);
        return enabled;
    }
    auto checked = joConfig["conditions"][objName]["checked"].toBool(false);
    cb->setChecked(checked);
    return checked;
}

QPushButton* ConditionFieldHandler::createDeleteConditionButton()
{
    QIcon deleteIcon(":/images/delete.png");

    auto button = new QPushButton;
    button->setObjectName("deleteConditionButton");
    button->setIcon(deleteIcon);
    button->setIconSize({8, 8});
    button->setFixedSize(12, 12);
    button->setStyleSheet("QPushButton { border: none; }");
    button->setToolTip(/*tr*/("Rimuovi condizione")); // TODO: tranlation here
    button->setCursor(Qt::PointingHandCursor);
    return button;
}
