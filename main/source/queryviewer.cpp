#include "queryviewer.h"
#include "ui_queryviewer.h"

#include "widgetsizetool.h"
#include "globals.h"

#include <QFileDialog>
#include <QClipboard>
#include <QJsonDocument>
#include <QDebug>

QueryViewer::QueryViewer(QWidget* parent) :
    QDialog(parent)
  , ui(new Ui::QueryViewer)
{
    auto dialog = Globals::createShadowedGui(this);
    ui->setupUi(dialog);

    new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, true);

    connectObjects();
}

QueryViewer::QueryViewer(QWidget* parent, QString text)
    : QueryViewer(parent)
{
    ui->textBrowser->setText(text);
}

QueryViewer::~QueryViewer()
{
    delete ui;
}

void QueryViewer::saveQuery()
{
    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Salva query"),
                QDir::currentPath(),
                "Txt files (*.txt) ;; SQL files (*.sql);;");
    if( !fileName.isNull() )
    {
        QFile f(fileName);
        if (f.open(QIODevice::WriteOnly))
        {
            f.write(ui->textBrowser->toPlainText().toUtf8());
            f.close();
        }
    }
}

void QueryViewer::pushButtonSave_clicked()
{
    saveQuery();
}

void QueryViewer::pushButtonClose_clicked()
{
    reject();
}

void QueryViewer::pushButtonCopy_clicked()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->textBrowser->toPlainText());
    emit messageMethodRequest(tr("Query copiata"));
}

void QueryViewer::textBrowser_textChanged()
{
    auto enabled = ui->textBrowser->toPlainText().isEmpty() == false;
    ui->pushButtonCopy->setEnabled(enabled);
    ui->pushButtonSave->setEnabled(enabled);
}

void QueryViewer::connectObjects()
{
    connect(ui->pushButtonSave , &QPushButton::clicked     , this, &QueryViewer::pushButtonSave_clicked );
    connect(ui->pushButtonClose, &QPushButton::clicked     , this, &QueryViewer::pushButtonClose_clicked);
    connect(ui->pushButtonCopy , &QPushButton::clicked     , this, &QueryViewer::pushButtonCopy_clicked );
    connect(ui->textBrowser    , &QTextBrowser::textChanged, this, &QueryViewer::textBrowser_textChanged);
}
