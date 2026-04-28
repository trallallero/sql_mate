#include "jsonviewer.h"
#include "ui_jsonviewer.h"

#include "globals.h"
#include "jsonmodel.h"
#include "widgetsizetool.h"

#include <QClipboard>
#include <QJsonDocument>

JsonViewer::JsonViewer()
    : ui(new Ui::JsonViewer)
{
}

JsonViewer::~JsonViewer()
{
    if (ui)
        delete ui;
}

QString JsonViewer::getName() const
{
    return QString("JsonViewer");
}

void JsonViewer::execute()
{
    if (!m_uiSetup)
    {
        auto dialog = createPluginGui(this);
        ui->setupUi(dialog);

        m_uiSetup = true;

        new WidgetSizeTool(this, ui->groupBoxZero, true);

        connectObjects();

        auto stylesheet = getData()["stylesheet"].toString();
        QWidget::setStyleSheet(stylesheet);
    }

    if(languageChanged())
    {
        ui->retranslateUi(this);
        setLanguageChanged(false);
    }

    m_data = getData()["value"].toString().toUtf8();

    JsonModel* model = new JsonModel(this);
    ui->treeView->setModel(model);
    model->fromJson(m_data);

    ui->treeView->expandAll();
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    exec();
}

bool JsonViewer::isEnabled()
{
    auto value = getData()["value"].toString().toUtf8();
    return QJsonDocument::fromJson(value).isNull() == false;
}

void JsonViewer::pushButtonClose_clicked()
{
    close();
}

void JsonViewer::toolButtonCopy_clicked()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(QJsonDocument::fromJson(m_data).toJson(QJsonDocument::Indented));
    m_messageMethod(tr("Valore copiato"));
}

void JsonViewer::connectObjects()
{
    connect(ui->pushButtonClose, &QPushButton::clicked, this, & JsonViewer::pushButtonClose_clicked);
    connect(ui->pushButtonCopy , &QPushButton::clicked, this, & JsonViewer::toolButtonCopy_clicked );
}
