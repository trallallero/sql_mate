#include "newcondition.h"
#include "ui_newcondition.h"

#include "globals.h"

#include <QCheckBox>
#include <QFile>
#include <QRegularExpression>
#include <QJsonArray>

auto LeftStr  = QT_TR_NOOP("Sinistra (max %1)");
auto RightStr = QT_TR_NOOP("Destra (max %1)");

NewCondition::NewCondition(QStringList usedConditions, QList<QGroupBox*> groupBoxes, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewCondition)
    , m_usedConditions(usedConditions)
    , m_groupBoxes(groupBoxes)
{
    auto dialog = Globals::createShadowedGui(this);
    ui->setupUi(dialog);

    connectObjects();
    setup();

    m_childCountLeft  = m_groupBoxes[0]->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();
    m_childCountRight = m_groupBoxes[1]->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();

    ui->radioButtonLeft ->setEnabled(m_childCountLeft  < Globals::maxNumberOfConditionsPerGroup());
    ui->radioButtonRight->setEnabled(m_childCountRight < Globals::maxNumberOfConditionsPerGroup());

    ui->radioButtonLeft ->setText(QString(LeftStr).arg(Globals::maxNumberOfConditionsPerGroup() - m_childCountLeft));
    ui->radioButtonRight->setText(QString(RightStr).arg(Globals::maxNumberOfConditionsPerGroup() - m_childCountRight));
}

NewCondition::~NewCondition()
{
    delete ui;
}

void NewCondition::pushButtonAdd_clicked()
{
    QList<QJsonObject> jos;

    int position = ui->radioButtonLeft->isChecked()
            ? m_groupBoxes[0]->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count()
            : m_groupBoxes[1]->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();

    for(auto& conditionName : m_conditionsSelected)
    {
        QJsonObject jo;

        QString condition = QString("AND %1 = '{%2}'").arg(m_allConditions[conditionName], conditionName);

        jo["title"     ] = conditionName;
        jo["value"     ] = "";
        jo["conditions"] = QJsonArray::fromStringList({condition});
        jo["page"      ] = ui->radioButtonLeft->isChecked() ? 0 : 1;
        jo["position"  ] = position++;
        jo["not"       ] = false;
        jo["in"        ] = false;
        jo["like"      ] = false;

        jos.append(jo);
    }

    emit conditionsSelected(jos, ui->lineEditJoinName->text().trimmed());
    close();
}

void NewCondition::pushButtonCancel_clicked()
{
    reject();
}

void NewCondition::setup()
{
    QFile file_query(Globals::filenameProfileQuery());
    file_query.open(QIODevice::ReadOnly | QIODevice::Text);
    auto query = QString(file_query.readAll());
    file_query.close();

    QRegularExpression re("^(.*) AS \"(.*)\".*$");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator i = re.globalMatch(query);
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        QString field = match.captured(1).trimmed();
        QString name  = match.captured(2).trimmed();
        if (m_usedConditions.contains(name.toLower()) == false && name.toLower() != "tenant")
            m_allConditions[name] = field;
    }

    for(auto& item : m_allConditions.toStdMap())
    {
        auto cb = new QCheckBox(item.first, ui->groupBoxZeroConditionNames);
        cb->setCursor(Qt::PointingHandCursor);
        cb->setToolTip(item.second);
        cb->setObjectName("checkBoxCondition");
        connect(cb, &QCheckBox::toggled, this, [this, cb](bool checked){
            if (checked)
                m_conditionsSelected.append(cb->text());
            else
                m_conditionsSelected.removeOne(cb->text());
            ui->pushButtonAdd->setEnabled(canPressAdd());
            ui->checkBoxUnite->setEnabled(canUnite());
        });
        ui->groupBoxZeroConditionNames->layout()->addWidget(cb);
        m_allCheckBoxes.append(cb);
    }
    ui->checkBoxSelectAll->setEnabled(m_allCheckBoxes.count() > 0);
}

void NewCondition::connectObjects()
{
    connect(ui->lineEditJoinName, &QLineEdit::textChanged, this, [this](const QString&){
        ui->pushButtonAdd->setEnabled(canPressAdd());
    });

    connect(ui->pushButtonAdd    , &QPushButton::clicked , this, &NewCondition::pushButtonAdd_clicked    );
    connect(ui->pushButtonCancel , &QPushButton::clicked , this, &NewCondition::pushButtonCancel_clicked );
    connect(ui->checkBoxSelectAll, &QCheckBox::toggled   , this, &NewCondition::checkBoxSelectAll_toggled);
    connect(ui->checkBoxUnite    , &QCheckBox::toggled   , this, &NewCondition::checkBoxUnite_toggled    );
    connect(ui->radioButtonLeft  , &QRadioButton::clicked, this, &NewCondition::radioButtonLeft_clicked  );
    connect(ui->radioButtonRight , &QRadioButton::clicked, this, &NewCondition::radioButtonRight_clicked );
}

bool NewCondition::canPressAdd() const
{
    if(m_conditionsSelected.count() <= 0)
        return false;
    if (ui->radioButtonLeft->isChecked() == false && ui->radioButtonRight->isChecked() == false)
        return false;
    if (ui->checkBoxUnite->isChecked())
    {
        if(ui->lineEditJoinName->text().trimmed().isEmpty())
            return false;
        if(m_conditionsSelected.count() < 2)
            return false;

        if(ui->radioButtonLeft->isChecked() && m_childCountLeft >= Globals::maxNumberOfConditionsPerGroup())
            return false;
        if(ui->radioButtonRight->isChecked() && m_childCountRight >= Globals::maxNumberOfConditionsPerGroup())
            return false;
    }
    else
    {
        if(ui->radioButtonLeft->isChecked() && (m_conditionsSelected.count() + m_childCountLeft > Globals::maxNumberOfConditionsPerGroup()))
            return false;
        if(ui->radioButtonRight->isChecked() && (m_conditionsSelected.count() + m_childCountRight > Globals::maxNumberOfConditionsPerGroup()))
            return false;
    }

    return true;
}

bool NewCondition::canUnite() const
{
    auto childs = ui->groupBoxZeroConditionNames->findChildren<QCheckBox*>(QRegularExpression("checkBoxCondition"), Qt::FindDirectChildrenOnly);
    auto count = 0;
    for(auto& child : childs)
    {
        if(child->isChecked())
            ++count;
        if(count > 1)
            return true;
    }
    return false;
}

void NewCondition::checkBoxSelectAll_toggled(bool checked)
{
    for(auto& cb : m_allCheckBoxes)
        cb->setChecked(checked);
}

void NewCondition::radioButtonLeft_clicked()
{
    ui->pushButtonAdd->setEnabled(canPressAdd());
}

void NewCondition::radioButtonRight_clicked()
{
    ui->pushButtonAdd->setEnabled(canPressAdd());
}

void NewCondition::checkBoxUnite_toggled(bool checked)
{
    ui->lineEditJoinName->setEnabled(checked);
    if(checked)
        ui->lineEditJoinName->setFocus();
    ui->pushButtonAdd->setEnabled(canPressAdd());
}
