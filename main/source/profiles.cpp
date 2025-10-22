#include "profiles.h"
#include "ui_profiles.h"
#include "profiles.h"
#include "conditions.h"
#include "filesystemmodel.h"
#include "widgetsizetool.h"
#include "globals.h"

#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <QFileSystemModel>
#include <QDirModel>
#include <QMessageBox>
#include <QDebug>

Profiles::Profiles(QWidget* parent, QString schema)
    : QDialog(parent)
    , ui(new Ui::Profiles)
    , m_currentSchema(schema)
{
    m_dialog = Globals::createShadowedGui(this);
    ui->setupUi(m_dialog);

    new WidgetSizeTool(this, ui->groupBoxZeroMainButtons, false);

    connectObjects();

    installEventFilter(this);

    m_profilesDir = Globals::profilesDirectory().replace("/", "");

    auto dirModel = new FileSystemModel(this, Globals::currentProfile(), m_currentSchema);
    dirModel->setRootPath(Globals::profilesDirectory());
    dirModel->setFilter(QDir::Dirs| QDir::Files | QDir::NoDotAndDotDot);
    ui->treeView->setModel(dirModel);

    ui->treeView->setColumnHidden(1, true);
    ui->treeView->setColumnHidden(2, true);
    ui->treeView->setColumnHidden(3, true);

    auto idx = dirModel->index(Globals::profilesDirectory());
    ui->treeView->setRootIndex(idx);
    ui->treeView->setHeaderHidden(true);

    connect(dirModel, &QFileSystemModel::directoryLoaded, this, [this, dirModel, idx](){
        while(dirModel->canFetchMore(idx))
            dirModel->fetchMore(idx);

        auto idx = dirModel->index(Globals::currentProfileDirectory());
        ui->treeView->expand(idx);
        expandSchema(idx, m_currentSchema);
    });

    m_dialog->adjustSize();
}

Profiles::~Profiles()
{
    delete ui;
}

void Profiles::expandSchema(const QModelIndex& index, QString schema)
{
    if (!index.isValid() || schema.isEmpty())
        return;

    int childCount = index.model()->rowCount(index);
    for (int i = 0; i < childCount; ++i)
    {
        const QModelIndex& child = index.child(i, 0);
        // Recursively call the function for each child node.
        if(child.data().toString() == schema)
            expandSchema(child, schema);
    }

    ui->treeView->expand(index);
}

bool Profiles::saveCurrent(ElementType elementType, QJsonObject jsonObj, Conditions& conditions)
{
    auto filename = elementType == ET_Profile
            ? Globals::filenameProfileConditionFields()
            : Globals::filenameSchemaConditionFields();

    QFile fileConditions(filename);
    fileConditions.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream stream(&fileConditions);
    stream << "[" << endl;

    auto jsonObjList = conditions.getJsonConditions();
    for (auto it = jsonObjList.begin(); it != jsonObjList.end();)
    {
        QJsonDocument jd(*it);

        if (++it == jsonObjList.end())
            stream << jd.toJson(QJsonDocument::JsonFormat::Indented).trimmed();
        else
            stream << jd.toJson(QJsonDocument::JsonFormat::Indented).trimmed() << "," << endl;
    }
    stream << "]" << endl;

    fileConditions.close();

    auto jsonObjMap = conditions.getJsonConditionsValues();
    jsonObj["conditions"] = QJsonObject::fromVariantMap(jsonObjMap);

    filename = elementType == ET_Profile
            ? Globals::filenameProfileConfig()
            : Globals::filenameSchemaConfig();

    QByteArray ba = QJsonDocument(jsonObj).toJson(QJsonDocument::JsonFormat::Indented);
    QFile fout(filename );
    fout.open(QIODevice::WriteOnly);
    fout.write(ba);
    fout.close();

    return true;
}

bool Profiles::saveCurrentProfile(QJsonObject jsonObj, Conditions& conditions)
{
    return saveCurrent(ET_Profile, jsonObj, conditions);
}

bool Profiles::saveCurrentSchema(QJsonObject jsonObj, Conditions& conditions)
{
    if(QDir().exists(Globals::currentSchemaDirectory()) == false)
        QDir().mkdir(Globals::currentSchemaDirectory());

    return saveCurrent(ET_Schema, jsonObj, conditions);
}

QMap<int, QString> Profiles::getProfiles()
{
    QMap<int, QString> profiles;
    QDirIterator directories(Globals::profilesDirectory(), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    int index = 0;
    while(directories.hasNext())
    {
        directories.next();
        auto profile = directories.fileName();
        if(profile != Globals::templateProfileName())
            profiles[index++] = profile;
    }
    return profiles;
}

QMap<int, QString> Profiles::getSchemasForMenu()
{
    QMap<int, QString> schemas;
    auto schemasObj = Globals::profileSchemasObject();
    auto schemasList = schemasObj["schemas"].toVariant().toStringList();
    int index = 0;
    for(auto& schema : schemasList)
        schemas[index++] = schema;
    schemas[index++] = "separator";
    schemas[index++] = tr("<nuovo>");
    return schemas;
}

void Profiles::addSchema(QString schema)
{
    auto schemasObj = Globals::profileSchemasObject();
    auto schemasList = schemasObj["schemas"].toVariant().toStringList();
    if(schemasList.contains(schema))
        return;
    schemasList.append(schema);
    schemasObj["schemas"] = QJsonArray::fromStringList(schemasList);

    QByteArray ba = QJsonDocument(schemasObj).toJson(QJsonDocument::JsonFormat::Indented);
    QFile fout(Globals::filenameProfileSchemas());
    fout.open(QIODevice::WriteOnly);
    fout.write(ba);
    fout.close();
}

bool Profiles::profileExists(QString profile)
{
    return getProfiles().values().contains(profile);
}

QString Profiles::getFirstProfile()
{
   auto profiles = getProfiles();
   if (profiles.count() > 0)
       return profiles[0];
   return {};
}

void Profiles::updateProfileConnection(QString oldConnectionName, QString newConnectionName)
{
    auto profiles = getProfiles();
    for(const auto& profile : profiles.toStdMap())
    {
        auto fileName = Globals::filenameProfileConfig(profile.second);
        if (QFileInfo::exists(fileName))
        {
            QFile file(fileName);
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            auto jsonConfigDoc = QJsonDocument::fromJson(file.readAll());
            auto jsonConfigObj = jsonConfigDoc.object();
            if (jsonConfigObj.keys().contains("connection") && jsonConfigObj["connection"].toString() == oldConnectionName)
            {
                jsonConfigObj["connection"] = newConnectionName;
                file.close();
                file.open(QIODevice::WriteOnly | QIODevice::Text);
                file.write(QJsonDocument(jsonConfigObj).toJson());
                file.close();
            }
        }
    }
}

bool Profiles::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent *>(event);
        const auto modifiers = key->modifiers();
        if(modifiers & Qt::ControlModifier)
        {
            if(key->key() == Qt::Key_S)
            {
                pushButtonSave_clicked();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void Profiles::createProfile(QString profile)
{
    Globals::copyPath(Globals::profileTemplateDirectory(), Globals::profileDirectory(profile));
    m_profiles.append(profile);
    ui->lineEditNewProfile->clear();
    ui->pushButtonCreateProfile->setEnabled(false);
}

void Profiles::pushButtonCancel_clicked()
{
    close();
}

QString Profiles::getFileContent(QString filepath) const
{
    QString content;
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        content = file.readAll();
        file.close();
    }
    return content;
}

void Profiles::pushButtonSelect_clicked()
{
    emit profileSelected(m_profileSelected, ui->checkBoxDefault->isChecked());
    close();
}

void Profiles::pushButtonSave_clicked()
{
    QFile file(m_currentFilename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(ui->textEdit->toPlainText().replace("\t", "    ").toLocal8Bit());
        file.close();
        emit messageMethodRequest(tr("File salvato"));
    }
    ui->textEdit->setText(getFileContent(m_currentFilename));
}

void Profiles::pushButtonCreateProfile_clicked()
{
    auto profile = ui->lineEditNewProfile->text().trimmed();
    if(m_profiles.contains(profile))
        emit messageMethodRequest(tr("Profilo già esistente"));
    else
        createProfile(profile);
}

void Profiles::lineEditNewProfile_textChanged(const QString& arg1)
{
    ui->pushButtonCreateProfile->setEnabled(arg1.isEmpty() == false);
}

void Profiles::treeView_clicked(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    disconnect(ui->textEdit, &QTextEdit::textChanged, nullptr, nullptr);

    ui->pushButtonSave->setEnabled(false);
    ui->pushButtonUpdate->setEnabled(false);

    auto info = getInfoFromIndex(index);

    m_currentFilename       = info.first.second;
    m_currentEditingProfile = info.first.first;

    if (m_currentEditingProfile != Globals::currentProfile())
        ui->pushButtonUpdate->setEnabled(false);

    if(info.second == 1) // cliked root item
    {
        ui->groupBoxZeroSelect->setEnabled(true);
        m_profileSelected = index.data(Qt::DisplayRole).toString();
        ui->checkBoxDefault->setChecked(m_profileSelected == Globals::currentProfile());
        ui->checkBoxDefault->setEnabled(ui->checkBoxDefault->isChecked() == false);
    }
    else
    {
        ui->groupBoxZeroSelect->setEnabled(false);
        m_profileSelected = "";
    }

    if(QFileInfo(info.first.second).isFile())
    {
        ui->textEdit->setText(getFileContent(m_currentFilename));
        ui->textEdit->setEnabled(true);
        QTimer::singleShot(0, this, [this](){
            connect(ui->textEdit, &QTextEdit::textChanged, this, [this](){
                ui->pushButtonSave  ->setEnabled(true);
                ui->pushButtonUpdate->setEnabled(m_currentEditingProfile == Globals::currentProfile());
            });
        });
    }
    else
    {
        ui->textEdit->setText("");
        ui->textEdit->setEnabled(false);
    }
}

QPair<QPair<QString, QString>, int> Profiles::getInfoFromIndex(const QModelIndex& index) const
{
    QString currentEditingProfile;
    QString path;
    QModelIndex i = index;
    int level = 0;
    while(i.parent().isValid())
    {
        auto text = i.data(Qt::DisplayRole).toString();
        path.prepend("/" + text);
        if(text == m_profilesDir)
            break;
        currentEditingProfile = i.data(Qt::DisplayRole).toString();
        i = i.parent();
        ++level;
    }
    return { {currentEditingProfile, QDir::currentPath() + path}, level };
}

void Profiles::treeView_doubleClicked(const QModelIndex& index)
{
    auto info = getInfoFromIndex(index);
    if(info.second == 1) // double cliked root item
    {
        m_profileSelected = index.data(Qt::DisplayRole).toString();
        emit profileSelected(m_profileSelected, ui->checkBoxDefault->isChecked());
        close();
    }
}

void Profiles::pushButtonRemove_clicked()
{
    auto result = QMessageBox::question(this, tr("ATTENZIONE"), tr("Sicuro di voler eliminare il profilo") + QString(" <%1>?").arg(m_profileSelected),
                                        QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::Cancel);
    if (result == QMessageBox::StandardButton::Yes)
    {
        QDir(m_currentFilename).removeRecursively();
        ui->groupBoxZeroSelect->setEnabled(false);
    }
}

void Profiles::connectObjects()
{
    connect(ui->pushButtonCancel       , &QPushButton::clicked    , this, &Profiles::pushButtonCancel_clicked       );
    connect(ui->pushButtonSelect       , &QPushButton::clicked    , this, &Profiles::pushButtonSelect_clicked       );
    connect(ui->pushButtonSave         , &QPushButton::clicked    , this, &Profiles::pushButtonSave_clicked         );
    connect(ui->pushButtonUpdate       , &QPushButton::clicked    , this, &Profiles::currentProfileUpdate           );
    connect(ui->pushButtonCreateProfile, &QPushButton::clicked    , this, &Profiles::pushButtonCreateProfile_clicked);
    connect(ui->pushButtonRemove       , &QPushButton::clicked    , this, &Profiles::pushButtonRemove_clicked       );
    connect(ui->lineEditNewProfile     , &QLineEdit::textChanged  , this, &Profiles::lineEditNewProfile_textChanged );
    connect(ui->treeView               , &QTreeView::clicked      , this, &Profiles::treeView_clicked               );
    connect(ui->treeView               , &QTreeView::doubleClicked, this, &Profiles::treeView_doubleClicked         );
}
