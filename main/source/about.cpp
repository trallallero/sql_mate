#include "about.h"
#include "ui_about.h"
#include "globals.h"

#include <QDebug>

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    auto dialog = Globals::createShadowedGui(this);
    ui->setupUi(dialog);

    connect(ui->pushButtonClose, &QPushButton::clicked, this, [this](){ reject(); });
}

About::~About()
{
    delete ui;
}
