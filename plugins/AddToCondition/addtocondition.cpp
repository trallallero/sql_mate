#include "addtocondition.h"
#include "ui_addtocondition.h"

#include <QApplication>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QJsonArray>
#include <QDebug>

AddToCondition::AddToCondition()
    : ui(new Ui::AddToCondition)
{
}

AddToCondition::~AddToCondition()
{
    if (ui)
        delete ui;
}

QString AddToCondition::getName() const
{
    return QString("Add to condition");
}

void AddToCondition::execute()
{
    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        m_uiSetup = true;

        connectObjects();

        auto stylesheet = getData()["stylesheet"].toString();
        QWidget::setStyleSheet(stylesheet);
    }

    if(languageChanged())
    {
        ui->retranslateUi(this);
        setLanguageChanged(false);
    }

    auto value = getData()["value"].toString();
    ui->lineEditValue->setText(value);

    exec();
}

bool AddToCondition::isEnabled()
{
    auto title          = getData()["title"].toString();
    auto conditionNames = getData()["conditionNames"].toArray().toVariantList();
    return conditionNames.contains(title.toLower());
}

void AddToCondition::pushButtonCancel_clicked()
{
    close();
}

void AddToCondition::pushButtonOk_clicked()
{
    auto mainWidget = PluginInterface::getMainWidget();

    auto title = getData()["title"].toString();
    auto value = getData()["value"].toString();

    auto children = mainWidget->findChildren<QFrame*>();
    for(auto& child : children)
    {
        if (child->objectName().startsWith("groupBoxCondition_"))
        {
            auto cb = child->findChild<QCheckBox*>("checkBoxTitle");

            if (cb && cb->text().toLower() == title.toLower())
            {
                auto children2 = child->findChildren<QLineEdit*>();
                if (children2.size() == 0)
                {
                    m_messageMethod(tr("Condizione non trovata"));
                    close();
                    return;
                }

                if (children2.size() > 1)
                {
                    m_messageMethod(tr("Condizione multipla non compatibile"));
                    close();
                    return;
                }

                auto text = children2[0]->text();
                if (text.contains(value))
                {
                    if (cb->isChecked())
                    {
                        m_messageMethod(tr("Condizione già presente"));
                        close();
                        return;
                    }
                }
                else
                {
                    text += " " + value;
                    children2[0]->setText(text.trimmed());
                }

                if (cb->isChecked() == false)
                    cb->setChecked(true);

                auto children3 = child->findChildren<QCheckBox*>();
                for(auto& child3 : children3)
                {
                    if (child3->objectName() == "cbNot")
                        child3->setChecked(ui->checkBoxNOT->isChecked());
                    else if (child3->objectName() == "cbNull")
                        child3->setChecked(ui->checkBoxNULL->isChecked());
                    else if (child3->objectName() == "cbLike")
                        child3->setChecked(ui->checkBoxLIKE->isChecked());
                }
                if(ui->checkBoxExecute->isChecked())
                    m_searchMethod();
                close();
                return;
            }
        }
    }
    m_messageMethod(tr("Condizione non trovata o condizione multipla non compatibile"));
    close();
}

void AddToCondition::checkBoxNULL_toggled(bool checked)
{
    ui->checkBoxLIKE->setEnabled(!checked);
}


void AddToCondition::checkBoxLIKE_toggled(bool checked)
{
    ui->checkBoxNULL->setEnabled(!checked);
}

void AddToCondition::connectObjects()
{
    connect(ui->lineEditValue, &QLineEdit::textChanged, this, [this](const QString& text){
        if(text.isEmpty() == false)
            ui->checkBoxNULL->setChecked(false);
        ui->checkBoxNULL->setEnabled(text.isEmpty());
    });

    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &AddToCondition::pushButtonCancel_clicked);
    connect(ui->pushButtonOk    , &QPushButton::clicked, this, &AddToCondition::pushButtonOk_clicked    );

    connect(ui->checkBoxNULL, &QCheckBox::toggled, this, &AddToCondition::checkBoxNULL_toggled);
    connect(ui->checkBoxLIKE, &QCheckBox::toggled, this, &AddToCondition::checkBoxLIKE_toggled);
}

