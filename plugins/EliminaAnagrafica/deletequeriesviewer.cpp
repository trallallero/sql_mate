#include "deletequeriesviewer.h"
#include "ui_deletequeriesviewer.h"

#include "widgetsizetool.h"
#include "globals.h"

#include <QClipboard>

DeleteQueriesViewer::DeleteQueriesViewer(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DeleteQueriesViewer)
{
    auto dialog = Globals::createShadowedGui(this, true);
    ui->setupUi(dialog);

    new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, true);

    connect(ui->pushButtonClose, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->pushButtonCopy , &QPushButton::clicked, this, &DeleteQueriesViewer::copyText);
}

DeleteQueriesViewer::~DeleteQueriesViewer()
{
    delete ui;
}

void DeleteQueriesViewer::appendText(QString text)
{
    ui->textBrowser->append(text);
}

void DeleteQueriesViewer::clear()
{
    ui->textBrowser->clear();
}

void DeleteQueriesViewer::copyText()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->textBrowser->toPlainText());
    emit message(tr("Testo copiato"));
}
