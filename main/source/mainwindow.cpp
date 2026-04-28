#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "filterfields.h"
#include "queryviewer.h"
#include "exportresult.h"
#include "profiles.h"
#include "volatilemessage.h"
#include "newcondition.h"
#include "connections.h"
#include "dragframe.h"
#include "popupmenu.h"
#include "widgetsizetool.h"
#include "about.h"

#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QScreen>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDirIterator>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QMimeData>
#include <QEventLoop>

// TODO: on set profile, create a current profile configuration object from Globals::profileConfigurationObject()
// TODO: move to table view model instead of table widget
// TODO: make text translatable
// TODO: united conditions should be limited calculating how many conditions are united. The size of 2 united is similar to 1.5 single
// TODO: move code to globals when possible
// TODO: connect all auto connect methods (on_...) with a real connect call, it's safier and less error prone
// TODO: add feature rename profile/schema

auto updateSelectAllTenantsCheckBox = [](QCheckBox* cb, QList<QCheckBox*>& tenantsCheckBoxes){
    auto totChecked = 0;
    for (auto& p : tenantsCheckBoxes)
    {
        if (p->isChecked())
            ++totChecked;
    }
    if (totChecked == 0)
        cb->setChecked(false);
    if (totChecked == tenantsCheckBoxes.size())
        cb->setChecked(true);
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_conditions(this)
    , m_pluginsHandler(this)
{
    qRegisterMetaType<SqlResultType>();
    qRegisterMetaType<TenantQueryMap>();
    qRegisterMetaType<Qt::DropAction>();
    qRegisterMetaType<QVector<int>>();

    Globals::initialize();

    setAcceptDrops(true);
    setMouseTracking(true);

    ui->setupUi(this);
    ui->comboBoxConnection->addItems(Connections::getConnectionNames()); // m_setupFinished has to be false here to avoid signals
    m_setupFinished = true;

    createMenu();

    setupPlugins();

    Globals::setApplicationStyleSheet(qApp);

    finishSetup();

    setProfile(Globals::getProfileSetToDefault(), false, true);

    connectObjects();

    Globals::setOnConfigurationSaved(&MainWindow::onConfigurationSaved);

    ui->tableWidgetResult->setVisible(false);
    ui->tableView->show();
}

MainWindow::~MainWindow()
{
    m_pluginsHandler.unloadPlugins();
    delete ui;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    m_dropAccepted = false;

     if(watched == ui->groupBoxSearch || watched == ui->groupBoxSearch2)
    {
        if(event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove)
        {
            auto watchedGb = qobject_cast<QGroupBox*>(watched);

            m_dropAccepted =
                    watchedGb->layout()->count() <= Globals::maxNumberOfConditionsPerGroup()
                  || m_currentDragGroupBox == watchedGb;
            if(m_dropAccepted)
                m_currentDropGroupBox = watchedGb;
            else
                m_currentDropGroupBox = nullptr;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    event->setAccepted(m_dropAccepted);
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    QRect updateRect = m_highlightedRect.united(targetSquare(event->pos()));

    if (event->mimeData()->hasFormat("application/x-dnditemdata"))
    {
        m_highlightedRect = targetSquare(event->pos());
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        m_highlightedRect = QRect();
        event->ignore();
    }
    update(updateRect);
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
    QRect updateRect = m_highlightedRect;
    m_highlightedRect = QRect();
    update(updateRect);
    event->accept();
}

void MainWindow::dropEvent(QDropEvent*)
{
    if(m_currentDropGroupBox && m_currentDropFrame)
    {
        auto page = m_currentDropGroupBox == ui->groupBoxSearch ? 0 : 1;
        if(m_conditions.moveFrameTo(m_currentDropFrame, m_currentDropGroupBox, page) == false)
        {   // frame was not moved (same page) so update condition separator
            auto count1 = ui->groupBoxSearch ->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();
            auto count2 = ui->groupBoxSearch2->findChildren<QFrame*>(QRegularExpression("groupBoxCondition_.*"), Qt::FindDirectChildrenOnly).count();
            ui->lineConditionsSeparator->setVisible(count1 > 0 && count2 > 0);
        }
    }
    m_currentDropFrame    = nullptr;
    m_currentDropGroupBox = nullptr;
}

void MainWindow::on_pushButtonZeroSearch_clicked()
{
    if (m_sql.isFetching())
    {
        ui->groupBoxZeroInnerSearch->setEnabled(false);
        m_sql.abort();
        showVolatileMessage(tr("Sto annullando l'operazione..."));
        return;
    }

    const auto conditions = m_conditions.getConditions().trimmed();

    if (conditions.isEmpty() && ui->comboBoxLimitSearch->currentText().isEmpty())
    {
        const auto answer = QMessageBox::warning(
            this,
            tr("ATTENZIONE!"),
            tr("L'esecuzione di una query senza condizioni potrebbe richiedere molto tempo.\n"
               "Sei sicuro di voler continuare?"),
            QMessageBox::Yes,
            QMessageBox::No);
        if (answer == QMessageBox::No)
            return;
    }

    const auto enabledTenants = getEnabledTenants();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    enableWidgetObjects(false, enabledTenants.count() > 1);
    m_sql.getResult(enabledTenants, conditions, ui->comboBoxLimitSearch->currentText(), ui->checkBoxZeroSearch->isChecked() == false);
}

void MainWindow::tenant_checked(bool toggled)
{
    if (toggled)
    {
        ui->groupBoxZeroInnerSearch->setEnabled(true);
        return;
    }
    for(auto& cb : m_tenantsCheckBoxes)
    {
        if (cb->isChecked())
        {
            ui->groupBoxZeroInnerSearch->setEnabled(true);
            return;
        }
    }
    ui->groupBoxZeroInnerSearch->setEnabled(false);
}

void MainWindow::applyProfile()
{
    resetTableWidget();
    m_sql.setQuery();
    m_conditions.addConditionFields({ui->groupBoxSearch, ui->groupBoxSearch2});

    m_selectedFields = FilterFields::loadProfileSelectedFields();
    setProfileTenantsChecked();

    //ui->checkBoxZeroDefaultProfile->setChecked(Globals::isCurrentProfileDefault());

    setWindowTitle(Globals::applicationTitle());
}

void MainWindow::setProfileTenantsChecked()
{
    auto jsonObj         = Globals::profileConfigurationObject();
    auto selectedTenants = jsonObj["selectedTenants"].toVariant().toStringList();
    auto children        = ui->groupBoxTenantsList->children();

    for (auto& child : children)
    {
        if (child->objectName().isEmpty() == false)
            continue;

        auto cb = static_cast<QCheckBox*>(child);
        if (cb)
        {
            if (selectedTenants.contains(cb->text()))
                cb->setChecked(true);
            else
                cb->setChecked(false);
        }
    }
}

void MainWindow::setProfileConnection(bool force)
{
    auto jsonObj    = Globals::profileConfigurationObject();
    auto connection = jsonObj["connection"].toString();

    // should happen only on startup if connection combo has already the connection text so we have to force
    force = force && ui->comboBoxConnection->currentText() == connection;

    ui->comboBoxConnection->setCurrentText(connection); // NOTE: this will initialize sql thus set m_connectedToDb
    if(force)
        on_comboBoxConnection_currentIndexChanged(connection);
}

void MainWindow::setProfile(QString profile, bool isDefault, bool force)
{
    if (Globals::currentProfile() == profile && force == false)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (Profiles::profileExists(profile) == false)
        profile = Profiles::getFirstProfile(); // fallback in case of non existing profile


    // bk current if new profile fails
    auto currentProfile = Globals::currentProfile();
    auto currentSchema  = Globals::currentSchema();

    Globals::setProfile(profile);
    Globals::setSchema(Globals::getSchemaSetToDefault());

    setProfileConnection(force);

    if(m_connectedToDb == false)
    {
        QApplication::restoreOverrideCursor();
        Globals::setProfile(currentProfile);
        Globals::setSchema (currentSchema);
        setProfileConnection(false);
        return;
    }

    if(isDefault || /*force ||*/ Globals::getProfileSetToDefault() == profile)
    {
        Globals::setProfileToDefault(profile);
        ui->checkBoxZeroDefaultProfile->setChecked(true);
    }
    else
    {
        ui->checkBoxZeroDefaultProfile->setChecked(false);
    }

    ui->labelSchema->setText(Globals::currentSchema());

    ui->checkBoxZeroDefaultSchema->setChecked(Globals::isCurrentSchemaDefault());
    ui->checkBoxZeroDefaultSchema->setEnabled(ui->checkBoxZeroDefaultSchema->isChecked() == false && Globals::currentSchema().isEmpty() == false);

    m_lastFocusWidget = nullptr;
    m_conditions.removeConditionFields();

    setProfileQueryLimit();
    addTenantCheckBoxes();
    applyProfile();

    QApplication::restoreOverrideCursor();
}

void MainWindow::showVolatileMessage(const QString message, bool infinite)
{
    VolatileMessage* vm = new VolatileMessage(this, message);
    vm->show(infinite);
    vm->raise();
}

void MainWindow::addCondition(QJsonObject jo)
{
    m_conditions.addConditionField({ui->groupBoxSearch, ui->groupBoxSearch2}, jo);
}

void MainWindow::showConnections()
{
    Connections c(this);
    connect(&c, &Connections::connectionAdded, this, [this](QString connectionName){
        ui->comboBoxConnection->addItem(connectionName);
        m_pluginsHandler.setSqlDBs(Connections::getConnectionNames());
    });
    connect(&c, &Connections::connectionModified, this, [this](QString oldConnectionName, QString newConnectionName){
        auto index = ui->comboBoxConnection->findData(oldConnectionName, Qt::DisplayRole);
        if (index >= 0)
            ui->comboBoxConnection->setItemData(index, newConnectionName, Qt::DisplayRole);
        m_pluginsHandler.setSqlDBs(Connections::getConnectionNames());
        Profiles::updateProfileConnection(oldConnectionName, newConnectionName);
        if (m_sql.updateConnection(oldConnectionName, newConnectionName))
            m_pluginsHandler.setCurrentSqlDB(newConnectionName, m_sql.sqlDB());
    });
    connect(&c, &Connections::showMessage, this, [this](const QString msg){ showVolatileMessage(msg); });
    c.exec();
}

void MainWindow::showAbout()
{
    About(this).exec();
}

void MainWindow::sqlResultReady()
{
    if (ui->checkBoxZeroSearch->isChecked() == false)
        m_sqlResult.clear();

    ui->tableView->setModel(&m_sql.model(m_currentViewMode));
    ui->tableView->setTenantQueryMap(m_tenantQueryMap);
    ui->labelResultValue->setText(QString::number(m_sqlResult.count()).rightJustified(3, ' '));
    enableWidgetObjects(true, false);
    QApplication::restoreOverrideCursor();
}

void MainWindow::pluginExportResult(QWidget* parent, QTableWidget* tw, ViewMode vm)
{
    ExportResult er(parent, tw, vm);
    er.exec();
}

void MainWindow::setProfileQueryLimit() //  TODO: rename this method cos does more then
{
    auto jsonObj = Globals::profileConfigurationObject();
    auto limit   = jsonObj["limitQuery"].toString();
    ui->comboBoxLimitSearch->setCurrentText(limit);
    auto vm = jsonObj["viewMode"].toString();
    m_currentViewMode = vm == "v" ? VM_VERTICAL : VM_HORIZONTAL;
    ui->tableWidgetResult->setViewMode(m_currentViewMode);
    ui->tableView        ->setViewMode(m_currentViewMode);
    setViewModeTextToButton();
}

void MainWindow::queryByTenant(QString tenant, QString& query)
{
    query = m_tenantQueryMap.contains(tenant)
            ? m_tenantQueryMap[tenant]
            : QString();
}

QRect MainWindow::targetSquare(const QPoint& position) const
{
    return QRect(position.x() / 2 * 2, position.y() / 2 * 2, 2, 2);
}

void MainWindow::createMenu()
{
    connect(ui->actionConnections   , &QAction::triggered, this, &MainWindow::showConnections);
    connect(ui->actionAbout_SQL_mate, &QAction::triggered, this, &MainWindow::showAbout);

    QActionGroup* langGroup = new QActionGroup(ui->menuLang);
    langGroup->addAction(ui->actionItLang);
    langGroup->addAction(ui->actionEnLang);
    langGroup->setExclusive(true);

    connect(langGroup, &QActionGroup::triggered, this, [this](QAction* action){
        switchTranslator(m_translator, QString("SqlMate_%1.qm").arg(action->text()));
    });
}

void MainWindow::setupPlugins()
{
    connect(&m_pluginsHandler, &PluginsHandler::searchMethodRequest          , this, &MainWindow::on_pushButtonZeroSearch_clicked);
    connect(&m_pluginsHandler, &PluginsHandler::messageMethodRequest         , this, [this](const QString msg){ showVolatileMessage(msg); });
    connect(&m_pluginsHandler, &PluginsHandler::addConditionMethodRequest    , this, &MainWindow::addCondition);
    connect(&m_pluginsHandler, &PluginsHandler::setExportResultMethodRequest , this, &MainWindow::pluginExportResult);
    connect(&m_pluginsHandler, &PluginsHandler::setSqlConnectionMethodRequest, this, [this](QString connectionName, bool showMessage){
        m_showConnectionMessage = showMessage;
        ui->comboBoxConnection->setCurrentText(connectionName);
        m_showConnectionMessage = true;
    });
    connect(&m_pluginsHandler, &PluginsHandler::setSqlQueryMethodRequest, this, [this](QString tenant, QString query, QObject* sender){
        m_sql.getResultForPlugins(tenant, query, sender);
    });

    connect(&m_pluginsHandler, &PluginsHandler::setGetQueryMethodRequest, this, [this](QString tenant, QString& query){
        this->queryByTenant(tenant, query);
    });

    m_pluginsHandler.loadPlugins();

    m_pluginsHandler.setCurrentSqlDB(ui->comboBoxConnection->currentText(), m_sql.sqlDB());
    m_pluginsHandler.setSqlDBs(Connections::getConnectionNames());
    m_pluginsHandler.setData({{"stylesheet", styleSheet()}});
    ui->menuPlugins->addActions(m_pluginsHandler.getMainMenu()->actions());
}

void MainWindow::enableWidgetObjects(bool enable, bool canBeCanceled)
{
    ui->groupBoxTenants        ->setEnabled(enable);
    ui->groupBoxMainSearch     ->setEnabled(enable);
    ui->groupBoxResult         ->setEnabled(enable);

    if (enable)
    {
        ui->pushButtonZeroSearch->setText(tr("Cerca"));
        ui->groupBoxZeroInnerSearch->setEnabled(true);
        ui->groupBoxZeroInnerSearch->setProperty("conditionSearchButton", "idle");
    }
    else if (canBeCanceled)
    {
        ui->pushButtonZeroSearch->setText(tr("Annulla"));
        ui->groupBoxZeroInnerSearch->setEnabled(true);
        ui->groupBoxZeroInnerSearch->setProperty("conditionSearchButton", "searching");
        ui->pushButtonZeroSearch->setCursor(Qt::PointingHandCursor);
    }
    else
    {
        ui->groupBoxZeroInnerSearch->setEnabled(false);
        return;
    }

    ui->groupBoxZeroInnerSearch->style()->unpolish(ui->groupBoxZeroInnerSearch);
    ui->groupBoxZeroInnerSearch->style()->polish  (ui->groupBoxZeroInnerSearch);
}

void MainWindow::finishSetup()
{
    new WidgetSizeTool(this, ui->groupBoxZeroStatusBar, false);

    ui->labelSchema->setText(Globals::currentSchema());

    ui->groupBoxSearch->installEventFilter(this);
    ui->groupBoxSearch2->installEventFilter(this);

    ui->verticalLayoutSearch1->addStretch(3);
    ui->verticalLayoutSearch2->addStretch(3);

    ui->pushButtonZeroSearch->setText(tr("Cerca"));
    ui->groupBoxZeroInnerSearch->setProperty("conditionSearchButton", "idle");
    ui->groupBoxZeroInnerSearch->style()->unpolish(ui->groupBoxZeroInnerSearch);
    ui->groupBoxZeroInnerSearch->style()->polish  (ui->groupBoxZeroInnerSearch);
    on_tableWidgetResult_itemSelectionChanged();
    ui->labelResultValue->setText(QString::number(0).rightJustified(3, ' '));
}

void MainWindow::loadSchema()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_lastFocusWidget = nullptr;
    m_conditions.removeConditionFields();

    setProfileQueryLimit();
    setProfileConnection(false);
    applyProfile();

    QApplication::restoreOverrideCursor();
}

void MainWindow::changeEvent(QEvent* event)
{
    if(event != nullptr)
    {
        switch(event->type())
        {
            case QEvent::LanguageChange:
                ui->retranslateUi(this);
                on_tableWidgetResult_itemSelectionChanged();
                m_pluginsHandler.languageChanged();
                break;
            // TODO: case QEvent::LocaleChange:
            default:
                break;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::switchTranslator(QTranslator& translator, const QString& filename)
{
    qApp->removeTranslator(&translator);

    QString path = QApplication::applicationDirPath();
    if(translator.load(path + QDir::separator() + filename))
        qApp->installTranslator(&translator);
}

void MainWindow::setViewModeTextToButton()
{
    if (m_currentViewMode == ViewMode::VM_HORIZONTAL)
        ui->toolButtonSwitchView->setText(tr("Verticale"));
    else
        ui->toolButtonSwitchView->setText(tr("Orizzontale"));
}

QJsonObject MainWindow::getProfileSchemaJsonValues() const
{
    return
    {
        {"selectedTenants", QJsonArray::fromStringList(getEnabledTenants()) },
        {"selectedFields" , QJsonArray::fromStringList(m_selectedFields)    },
        {"connection"     , ui->comboBoxConnection->currentText()           },
        {"limitQuery"     , ui->comboBoxLimitSearch->currentText()          },
        {"viewMode"       , m_currentViewMode == VM_VERTICAL ? "v" : "h"    }
    };
}

void MainWindow::connectObjects()
{
    connect(ui->checkBoxZeroDefaultProfile, &QCheckBox::toggled, this, &MainWindow::checkBoxZeroDefaultProfile_toggled);

    connect(ui->tableWidgetResult, &TableWidget::visibleItemsCount, this, [this](int count){
        ui->pushButtonExportResult->setEnabled(count > 0);
    });

    connect(ui->tableWidgetResult->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int logicalIndex){
        if(m_currentViewMode != VM_HORIZONTAL)
            return; // cannot order columns in vertical view mode
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->tableWidgetResult->sortByColumn(logicalIndex);
        QApplication::restoreOverrideCursor();
    });

    connect(ui->tableWidgetResult, &TableWidget::contextMenuRequest, this, [this](QJsonObject o){
        o["conditionNames" ] = QJsonArray::fromStringList(m_conditions.getConditionNames(true, false));
        o["profile"        ] = Globals::currentProfile();
        o["fieldsFreeLeft" ] = Globals::maxNumberOfConditionsPerGroup() - m_conditions.getHandler()->getFieldsCountPerPage(0);
        o["fieldsFreeRight"] = Globals::maxNumberOfConditionsPerGroup() - m_conditions.getHandler()->getFieldsCountPerPage(1);
        m_pluginsHandler.setData(o);
        m_pluginsHandler.getContextMenuWidget()->popup();
    });

    connect(ui->tableView, &TableView::contextMenuRequest
            , this, [this](QJsonObject o){
        o["conditionNames" ] = QJsonArray::fromStringList(m_conditions.getConditionNames(true, false));
        o["profile"        ] = Globals::currentProfile();
        o["fieldsFreeLeft" ] = Globals::maxNumberOfConditionsPerGroup() - m_conditions.getHandler()->getFieldsCountPerPage(0);
        o["fieldsFreeRight"] = Globals::maxNumberOfConditionsPerGroup() - m_conditions.getHandler()->getFieldsCountPerPage(1);
        m_pluginsHandler.setData(o);
        m_pluginsHandler.getContextMenuWidget()->popup();
    });

    connect(&m_conditions, &Conditions::checkboxDeleted, this, [this](QObject* cb){
        if (cb == m_lastFocusWidget)
            m_lastFocusWidget = nullptr;
    });

    connect(&m_conditions, &Conditions::lineEditFocusRequest, this, [this](){
        if(m_lastFocusWidget)
            m_lastFocusWidget->setFocus();
    });

    connect(&m_conditions, &Conditions::lineEditEnterPressed, this, [this](){
        on_pushButtonZeroSearch_clicked();
    });

    connect(&m_conditions, &Conditions::maxNumberOfConditionsReached, this, [this](){
        showVolatileMessage(QString(tr("Raggiunto massimo numero di condizioni") + QString(" (%1)").arg(Globals::maxNumberOfConditions())));
    });

    connect(&m_conditions, &Conditions::visibleGroups, this, [this](int count){
        ui->lineConditionsSeparator->setVisible(count > 1);
    });

    connect(&m_conditions, &Conditions::dragStartedSignal, this, [this](QFrame* frame){
        m_currentDropFrame = frame;
        m_currentDragGroupBox = qobject_cast<QGroupBox*>(frame->parent());
        ui->lineConditionsSeparator->setVisible(true);
    });

    connect(&m_conditions, &Conditions::frameSelected, this, [this](QFrame* frame, int position, int total){
        if(frame == nullptr || total < 2)
        {
            ui->groupBoxZeroUpDown->setEnabled(false);
        }
        else
        {
            ui->groupBoxZeroUpDown->setEnabled(true);
            ui->toolButtonCondUp  ->setEnabled(position > 0);
            ui->toolButtonCondDown->setEnabled(position < (total - 1));
        }
    });

    connect(&m_conditions, &Conditions::showMessage, this, [this](const QString msg){ showVolatileMessage(msg); });

    connect(&m_sql, &Sql::queryExecuted, this, [this](const QString query){
        m_currentQuery = query;
    });

    connect(&m_sql, &Sql::resultReady, this, [this](const TenantQueryMap& tenantQueryMap){
        VolatileMessage::stop();
        m_tenantQueryMap = tenantQueryMap;
        sqlResultReady();
    });

    connect(&m_sql, &Sql::currentTenant, this, [this](const QString msg){ showVolatileMessage(msg, true); });

    connect(&m_sql, &Sql::sqlError, this, [this](const QString error){
        VolatileMessage::stop();
        m_connectedToDb = false;
        sqlResultReady();
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("ERRORE"), error, QMessageBox::StandardButton::Ok);
    });

    connect(&m_sql, &Sql::sqlAborted, this, [this](){
        VolatileMessage::stop();
        sqlResultReady();
        QApplication::restoreOverrideCursor();
        showVolatileMessage(tr("Operazione annullata!"));
    });

    connect(qApp, &QApplication::focusChanged, this, [this](QWidget* oldWidget, QWidget* newWidget){
        if (oldWidget && newWidget && QString(oldWidget->metaObject()->className()) == "QLineEdit")
        {
            m_lastFocusWidget = oldWidget;
            connect(m_lastFocusWidget, &QLineEdit::destroyed, this, [this](){
                if(m_lastFocusWidget)
                    disconnect(m_lastFocusWidget, &QLineEdit::destroyed, this, nullptr);
                m_lastFocusWidget = nullptr;
            });
        }
    });

    QList<QPushButton*> allButtons = this->findChildren<QPushButton*>();
    for (auto& button : allButtons)
    {
        connect(button, &QPushButton::clicked, this, [this](bool){
            if (m_lastFocusWidget)
                m_lastFocusWidget->setFocus();
        });
    }

    connect(ui->toolButtonConnections, &QToolButton::clicked, this, &MainWindow::showConnections);
}

QStringList MainWindow::getEnabledTenants() const
{
    QList<QString> tenants;

    for(auto& cb : m_tenantsCheckBoxes)
        if(cb->isChecked())
            tenants.append(cb->text());
    return tenants;
}

void MainWindow::addTenantCheckBoxes()
{
    QLayoutItem* child;
    while ((child = ui->groupBoxTenantsList->layout()->takeAt(0)) != 0)
    {
        ui->groupBoxTenants->layout()->removeWidget(child->widget());
        delete child->widget();
        delete child;
    }

    m_tenantsCheckBoxes.clear();

    auto tenants = m_sql.getTenants();
    for(auto& t : tenants)
    {
        auto cb = new QCheckBox(t, ui->groupBoxTenantsList);
        cb->setCursor(Qt::PointingHandCursor);
        ui->groupBoxTenantsList->layout()->addWidget(cb);
        m_tenantsCheckBoxes.append(cb);
        connect(cb, &QCheckBox::toggled, this, &MainWindow::tenant_checked);
        connect(cb, &QCheckBox::toggled, this, [this](bool){
            if (m_lastFocusWidget)
                m_lastFocusWidget->setFocus();

            updateSelectAllTenantsCheckBox(ui->checkBoxSelectTenants, m_tenantsCheckBoxes);
        });
    }
}

void MainWindow::resetTableWidget()
{    
    ui->labelResultValue->setText(QString::number(0).rightJustified(3, ' '));
    ui->tableWidgetResult->resetWidget();
    ui->pushButtonExportResult->setEnabled(false);
}

void MainWindow::on_pushButtonFF_clicked()
{
    FilterFields ff(this);
    connect(&ff, &FilterFields::fieldsSelected, this, [this](const QList<QString> fields){
        m_selectedFields = fields;
        ui->tableWidgetResult->filterResultFields(m_selectedFields);
    });
    ff.exec();
}

void MainWindow::on_pushButtonExportResult_clicked()
{
    ExportResult er(this, ui->tableWidgetResult, m_currentViewMode);
    er.exec();
}

void MainWindow::on_checkBoxSelectTenants_toggled(bool checked)
{
    for (auto& cb : m_tenantsCheckBoxes)
        cb->setChecked(checked);
}

void MainWindow::on_pushButtonViewQuery_clicked()
{
    QueryViewer qv(this, m_currentQuery);
    connect(&qv, &QueryViewer::messageMethodRequest, this, [this](const QString msg){ showVolatileMessage(msg); });
    qv.exec();
}

void MainWindow::on_pushButtonSetStyle_clicked()
{
    Globals::setApplicationStyleSheet(qApp);
    QApplication::processEvents();

    showVolatileMessage(tr("Stili applicati"));
}

void MainWindow::on_pushButtonZeroProfiles_clicked()
{
    Profiles p(this);
    connect(&p, &Profiles::profileSelected     , this, [this](const QString profile, bool isDefault){
        setProfile(profile, isDefault, false);
    }, Qt::QueuedConnection);
    connect(&p, &Profiles::currentProfileUpdate, this, [this](){ setProfile(Globals::currentProfile(), false, true); });
    connect(&p, &Profiles::messageMethodRequest, this, [this](const QString msg){ showVolatileMessage(msg); });
    p.exec();
}

void MainWindow::on_pushButtonZeroSchemas_clicked()
{
    Profiles p(this, Globals::currentSchema());
    connect(&p, &Profiles::profileSelected     , this, [this](const QString profile, bool isDefault){
        setProfile(profile, isDefault, false);
    }, Qt::QueuedConnection);
    connect(&p, &Profiles::messageMethodRequest, this, [this](const QString msg){ showVolatileMessage(msg); });
    p.exec();
}

void MainWindow::on_toolButtonSwitchView_clicked()
{    
    if (m_currentViewMode == ViewMode::VM_VERTICAL)
        m_currentViewMode = ViewMode::VM_HORIZONTAL;
    else
        m_currentViewMode = ViewMode::VM_VERTICAL;

    setViewModeTextToButton();
    ui->tableWidgetResult->setViewMode(m_currentViewMode);
    ui->tableView        ->setViewMode(m_currentViewMode);

    ui->tableWidgetResult->populate(m_sqlResult, m_selectedFields);
    ui->labelResultValue->setText(QString::number(m_sqlResult.count()).rightJustified(3, ' '));
    ui->tableView->setModel(&m_sql.model(m_currentViewMode));
}

void MainWindow::on_toolButtonReset_clicked()
{
    m_conditions.clearAll();
    m_conditions.deactivateAll();
}

void MainWindow::on_toolButtonDeactivate_clicked()
{
    m_conditions.deactivateAll();
}

void MainWindow::on_toolButtonAddCondition_clicked()
{
    auto separatorVisible = ui->lineConditionsSeparator->isVisible();
    ui->lineConditionsSeparator->setVisible(true);

    NewCondition nc(m_conditions.getConditionNames(true, false), { ui->groupBoxSearch, ui->groupBoxSearch2 }, this);

    connect(&nc, &NewCondition::conditionsSelected, this, [this](QList<QJsonObject> jos, QString joinName){
        QStringList names;
        for(auto& jo : jos)
        {
            m_conditions.addConditionField({{ui->groupBoxSearch, ui->groupBoxSearch2}}, jo);
            names.append(jo["title"].toString());
        }
        if(joinName.isEmpty() == false)
            m_conditions.uniteSelectedFields(names, joinName);
    });
    auto result = nc.exec();
    if(result != QDialog::Accepted)
        ui->lineConditionsSeparator->setVisible(separatorVisible);
}

void MainWindow::on_comboBoxConnection_currentIndexChanged(const QString& arg1)
{
    if(m_setupFinished == false)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto error = m_sql.initalize(Connections::getConnectionValues(arg1));

    QApplication::restoreOverrideCursor();

    if (error.isEmpty() == false)
    {
        m_connectedToDb = false;
        ui->groupBoxZeroInnerSearch->setEnabled(false);
        QMessageBox::critical(this, tr("ERRORE"), error, QMessageBox::StandardButton::Ok);
    }
    else
    {
        m_connectedToDb = true;
        if(m_showConnectionMessage)
            showVolatileMessage(QString(tr("Connessione a '%1' avvenuta con successo").arg(arg1)));
        ui->groupBoxZeroInnerSearch->setEnabled(true);
        m_pluginsHandler.setCurrentSqlDB(ui->comboBoxConnection->currentText(), m_sql.sqlDB());
    }
}

void MainWindow::on_comboBoxLimitSearch_currentIndexChanged(int)
{
    if (m_lastFocusWidget)
        m_lastFocusWidget->setFocus();
}

void MainWindow::on_pushButtonZeroSelectProfile_clicked()
{
    new PopupMenu(this, Profiles::getProfiles(), Globals::currentProfile(), [this](QString profile, bool isDefault){
        this->setProfile(profile, isDefault);
    });
}

void MainWindow::on_pushButtonZeroSelectSchema_clicked()
{
    new PopupMenu(this, Profiles::getSchemasForMenu(), Globals::currentSchema(), [this](QString schema, bool){
        if(schema != Globals::currentSchema())
        {
            ui->labelSchema->setText(schema);
            Globals::setSchema(schema);
            ui->checkBoxZeroDefaultSchema->setChecked(Globals::isCurrentSchemaDefault());
            ui->checkBoxZeroDefaultSchema->setEnabled(ui->checkBoxZeroDefaultSchema->isChecked() == false);
            if(Profiles::getSchemasForMenu().values().contains(schema))
                loadSchema();
        }
    });
}

void MainWindow::on_pushButtonZeroSaveSchema_clicked()
{
    Profiles::addSchema(ui->labelSchema->text());

    if (Profiles::saveCurrentSchema(getProfileSchemaJsonValues(), m_conditions))
        showVolatileMessage(tr("Schema salvato"));
}

void MainWindow::on_pushButtonZeroSaveProfile_clicked()
{
    if (Profiles::saveCurrentProfile(getProfileSchemaJsonValues(), m_conditions))
        showVolatileMessage(tr("Profilo salvato"));
}

void MainWindow::on_tableWidgetResult_itemSelectionChanged()
{
    auto cellTot = QString::number(ui->tableWidgetResult->selectedItems().count()).rightJustified(3, ' ');
    ui->labelSelectedItemsValue->setText(QString("%1").arg(cellTot));

    QMap<int, int> rows, cols;
    for(auto& item : ui->tableWidgetResult->selectedItems())
    {
        rows[item->row()   ] = 1;
        cols[item->column()] = 1;
    }

    auto rowsTot = QString::number(rows.count()).rightJustified(3, ' ');
    auto colsTot = QString::number(cols.count()).rightJustified(3, ' ');
    ui->labelSelectedRowsValue->setText(QString("%1").arg(rowsTot));
    ui->labelSelectedColsValue->setText(QString("%1").arg(colsTot));
}

void MainWindow::checkBoxZeroDefaultProfile_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBoxZeroDefaultProfile->setEnabled(false);

        if (Globals::getProfileSetToDefault() != Globals::currentProfile())
        {
            Globals::setProfileToDefault(Globals::currentProfile());
            showVolatileMessage(tr("Profilo impostato come predefinito"));
        }
    }
    else
    {
        ui->checkBoxZeroDefaultProfile->setEnabled(true);
    }
}

void MainWindow::on_checkBoxZeroDefaultSchema_toggled(bool checked)
{
    if(checked)
    {
        ui->checkBoxZeroDefaultSchema->setEnabled(false);
        if (Globals::getSchemaSetToDefault() != ui->labelSchema->text())
        {
            Globals::setSchemaToDefault(ui->labelSchema->text());
            showVolatileMessage(tr("Schema impostato come predefinito"));
        }
    }
}

void MainWindow::on_toolButtonDeleteAll_clicked()
{
    auto result = QMessageBox::question(this, tr("ATTENZIONE"), tr("Sicuro di voler eliminare tutte le condizioni?"),
                                        QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::Cancel);
    if (result == QMessageBox::StandardButton::Yes)
        m_conditions.removeConditionFields();
}

void MainWindow::on_toolButtonCondUp_clicked()
{
    m_conditions.moveCurrentField(Conditions::FD_UP);
}

void MainWindow::on_toolButtonCondDown_clicked()
{
    m_conditions.moveCurrentField(Conditions::FD_DOWN);
}

void MainWindow::onConfigurationSaved(QWidget* w, QString message)
{
    VolatileMessage* vm = new VolatileMessage(w, message);
    vm->show();
    vm->raise();
}
