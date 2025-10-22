#include "conditionscontextmenu.h"

#include "globals.h"

#include <QToolButton>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>

ConditionsContextMenu::ConditionsContextMenu(QWidget* parent)
    : QMenu(parent)
{
    m_shadowMenuWidget = new ContextMenuWidget(&m_contextMenu);

    setObjectName("contextMenu_0");

    // Set object name for stylesheet
    m_contextMenu     .setObjectName("contextMenu_1");
    m_uniteFieldsMenu .setObjectName("contextMenu_subMenu_1");
    m_extractFieldMenu.setObjectName("contextMenu_subMenu_2");
    m_renameFieldMenu .setObjectName("contextMenu_subMenu_3");

    m_renameLineEdit.setParent(this);
    m_uniteLineEdit .setParent(this);

    m_renameLineEdit.setObjectName("lineEditMenu_1");
    m_uniteLineEdit .setObjectName("lineEditMenu_2");

    m_uniteFieldsMenu .setTitle(tr("Unisci"));
    m_extractFieldMenu.setTitle(tr("Estrai"));
    m_renameFieldMenu .setTitle(tr("Rinomina"));

    connect(&m_uniteFieldsMenu, &QMenu::aboutToShow, this, [this](){
        m_uniteLineEdit.selectAll();
        m_uniteLineEdit.setFocus();
    });

    connect(&m_renameFieldMenu, &QMenu::aboutToShow, this, [this](){
        m_renameLineEdit.selectAll();
        m_renameLineEdit.setFocus();
    });

    m_uniteTitleAction = new QWidgetAction(this);
    m_uniteTitleAction->setDefaultWidget(createWidget(&m_uniteLineEdit, &ConditionsContextMenu::uniteFields));

    m_renameTitleAction = new QWidgetAction(this);
    m_renameTitleAction->setDefaultWidget(createWidget(&m_renameLineEdit, &ConditionsContextMenu::renameField));

    m_renameFieldMenu.addAction(m_renameTitleAction);

    m_contextMenu.addMenu(&m_extractFieldMenu);
    m_contextMenu.addMenu(&m_uniteFieldsMenu);
    m_contextMenu.addMenu(&m_renameFieldMenu);

    connect(&m_uniteLineEdit, &QLineEdit::returnPressed, this, [this](){
        if(m_uniteLineEdit.text().isEmpty() == false)
        {
            emit uniteFields(m_frame, m_uniteLineEdit.text());
            m_contextMenu.hide();
        }
    });

    connect(&m_renameLineEdit, &QLineEdit::returnPressed, this, [this](){
        if(m_renameLineEdit.text().isEmpty() == false)
        {
            emit renameField(m_frame, m_renameLineEdit.text());
            m_contextMenu.hide();
        }
    });
}

void ConditionsContextMenu::showContextMenu(DragFrame* frame, const ConditionField& field, QMap<QCheckBox*, ConditionField>& cbFieldMap)
{
    m_frame = frame;

    auto map = cbFieldMap.toStdMap();
    auto enabledCount = std::count_if(map.begin(), map.end(), [](const std::pair<QCheckBox*, ConditionField>& p){ return p.first->isChecked(); });
    if(enabledCount > 1)
        m_uniteFieldsMenu.addAction(m_uniteTitleAction);
    else
        m_uniteFieldsMenu.removeAction(m_uniteTitleAction);

    auto title = field.title();
    m_uniteLineEdit.setText(title);
    m_renameLineEdit.setText(title);
    m_extractFieldMenu.clear();
    m_extractFieldMenu.setEnabled(false);
    auto conditionCount = field.conditionsCount();
    if(conditionCount > 1)
    {
        auto childs = frame->findChildren<QLabel*>();
        for(auto& child : childs)
            m_extractFieldMenu.addAction(child->text(), this, [this, child, frame](){ emit extractField(frame, child->text()); });
        if(childs.count() > 0)
        {
            m_extractFieldMenu.setEnabled(true);
            m_extractFieldMenu.addSeparator();
            m_extractFieldMenu.addAction(tr("Tutti"), this, [this, frame](){ emit extractField(frame, ""); });
        }
    }
    auto cb = frame->findChild<QCheckBox*>();
    m_uniteFieldsMenu.setEnabled(cb && cb->isChecked() && title.isEmpty() == false && enabledCount > 1);
    m_shadowMenuWidget->popup();
}

QWidget* ConditionsContextMenu::createWidget(QLineEdit* lineEdit, void (ConditionsContextMenu::*method)(DragFrame*, QString))
{
    auto hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 3, 0);

    lineEdit->setLayout(hLayout);

    auto tb = new QToolButton(this);
    tb->setText(tr("Ok"));
    tb->setCursor(Qt::PointingHandCursor);
    connect(tb, &QToolButton::clicked, this, [this, method, lineEdit](){
        (this->*method)(m_frame, lineEdit->text());
        m_contextMenu.hide();
    });
    connect(lineEdit, &QLineEdit::textChanged, this, [tb](const QString& text){
        tb->setEnabled(text.isEmpty() == false);
    });
    lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    hLayout->insertItem(0, new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(tb);
    m_contextMenu.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    return lineEdit;
}
