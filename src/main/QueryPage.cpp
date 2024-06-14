// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "QueryPage.h"
#include "ui_QueryPage.h"

#include "Application.h"
#include "DataModel.h"
#include "EndpointConfig.h"
#include "EndpointConfigModel.h"
#include "EndpointSelector.h"
#include "MainWindow.h"
#include "MlClientTools.h"
#include "Tools.h"
#include "UserSettings.h"
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

namespace {

const auto CfgPageSplitter= QString("Window/QueryPage/Splitter");

} // namespace

QueryPage::QueryPage(QWidget *parent) :
    QWidget{parent},
    ui{new Ui::QueryPage{}},
    possibleMatchesModel_{new DataModel{this}}
{
    ui->setupUi(this);
}

QueryPage::~QueryPage()
{
    saveWidgetState();

    delete ui;
}

void QueryPage::initialize(MainWindow* mainWindow)
{
    mainWindow_ = mainWindow;

    ui->queryResultPane->setVisible(false);
    ui->possibleMatchesPane->setVisible(false);

    ui->possibleMatches->setModel(possibleMatchesModel_);
    ui->possibleMatches->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mainWindow_, &MainWindow::endpointConfigChanged, this, &QueryPage::onEndpointConfigChanged);
    connect(mainWindow_, &MainWindow::selectedEndpointChanged, this, &QueryPage::onSelectedEndpointChanged);

    connect(ui->executeBtn, &QAbstractButton::clicked, this, &QueryPage::onExecuteButtonClicked);
    connect(ui->editPatientBtn, &QAbstractButton::clicked, this, &QueryPage::onEditPatientBtnClicked);
    connect(ui->copyPidBtn, &QAbstractButton::clicked, this, &QueryPage::onCopyPidBtnClicked);

    connect(ui->createAnywayBtn, &QAbstractButton::clicked, this, &QueryPage::onCreateAnywayBtnClicked);

    connect(ui->possibleMatches, &QTableView::doubleClicked, this, &QueryPage::onPossibleMatchesDoubleClicked);

    connect(ui->possibleMatches, &QTableView::customContextMenuRequested, this, &QueryPage::onCustomMenuRequested);

    loadWidgetState();
}

void QueryPage::loadWidgetState()
{
    UserSettings s;

    ui->splitter->restoreState(s.value(CfgPageSplitter).toByteArray());
}

void QueryPage::saveWidgetState()
{
    UserSettings s;

    s.setValue(CfgPageSplitter, ui->splitter->saveState());
}

void QueryPage::updateUiState()
{
    bool endpointSelected = mainWindow_->endpointSelector()->selectedEndpoint() != -1;

    ui->executeBtn->setEnabled(endpointSelected);
}

void QueryPage::execute(bool sureness)
{
    setEnabled(false);

    auto mlClient = createMlClient(mainWindow_->endpointSelector()->selectedEndpoint(),
                                   mainWindow_->endpointSelector()->currentApiKey(),
                                   mainWindow_, &MainWindow::logMessage);
    mlClientQueryPatientData(mlClient, ui->patientDataForm->extractFormData(), sureness,
                             this, &QueryPage::onPatientDataQueringDone);
}

void QueryPage::reloadDynamicForm(int endpointIndex)
{
    QList<DynamicForm::Field> dynamicFields;

    if (endpointIndex != -1)
    {
        const auto model = app()->endpointConfigModel();
        const auto modelIndex = model->index(endpointIndex, toInt(EndpointConfig::Field::Fields));
        const auto fields = model->data(modelIndex, Qt::DisplayRole).toStringList();

        for (const auto& field : fields)
        {
            dynamicFields << DynamicForm::Field{field};
        }
    }

    ui->patientDataForm->reset(dynamicFields);
}

void QueryPage::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);

    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}

void QueryPage::onEndpointConfigChanged()
{
    reloadDynamicForm(mainWindow_->endpointSelector()->selectedEndpoint());
}

void QueryPage::onSelectedEndpointChanged(int index)
{
    reloadDynamicForm(index);

    updateUiState();
}

void QueryPage::onExecuteButtonClicked()
{
    execute(false);
}

void QueryPage::onEditPatientBtnClicked()
{
    mainWindow_->openPage(MainWindow::Page::Editor, ui->patientPid->text());
}

void QueryPage::onCopyPidBtnClicked()
{
    QGuiApplication::clipboard()->setText(ui->patientPid->text());
    mainWindow_->showStatusMessage(tr("Copied PID to Clipboard"), 1000);
}

void QueryPage::onCreateAnywayBtnClicked()
{
    execute(true);
}

void QueryPage::onCustomMenuRequested(const QPoint& position)
{
    const auto index = ui->possibleMatches->indexAt(position);
    if (!index.isValid())
        return;

    QMenu ctxMenu;

    QAction copyItemAction{QIcon::fromTheme(QStringLiteral("edit-copy")), tr("&Copy value")};
    connect(&copyItemAction, &QAction::triggered, this, [index]() {
        auto* clipboard = QGuiApplication::clipboard();
        clipboard->setText(index.data().toString());
    });
    ctxMenu.addAction(&copyItemAction);

    QAction copyRowAction{QIcon::fromTheme(QStringLiteral("edit-copy")), tr("&Copy entire row")};
    connect(&copyRowAction, &QAction::triggered, this, [index, model=possibleMatchesModel_]() {
        auto* clipboard = QGuiApplication::clipboard();
        const auto rowData = model->modelData()[index.row()].join(QLatin1Char(';'));
        clipboard->setText(rowData);
    });
    ctxMenu.addAction(&copyRowAction);

    ctxMenu.exec(ui->possibleMatches->viewport()->mapToGlobal(position));
}

void QueryPage::onPatientDataQueringDone(const MlClient::Error& error, const MlClient::QueryResult& result)
{
    if (error)
    {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("Error while quering patient data: %1").arg(error.message),
                    QMessageBox::Ok,
                    QMessageBox::Ok);

        mainWindow_->showStatusMessage(tr("Failed to query Patient data"), 5000);
    }
    else
    {
        if (!result.pid.isNull())
        {
            ui->patientPid->setText(result.pid);

            ui->queryResultPane->setVisible(true);
            ui->possibleMatchesPane->setVisible(false);
        }
        else
        {
            ui->possibleMatches->setEnabled(false);

            ui->queryResultPane->setVisible(false);
            ui->possibleMatchesPane->setVisible(true);

            auto mlClient = createMlClient(mainWindow_->endpointSelector()->selectedEndpoint(),
                                           mainWindow_->endpointSelector()->currentApiKey(),
                                           mainWindow_, &MainWindow::logMessage);
            mlClientLoadPatientData(mlClient, result.possibleMatchPids,
                                    mainWindow_->endpointSelector()->currentFieldList(),
                                     this, &QueryPage::onPatientDataLoadingDone);
        }

        mainWindow_->showStatusMessage(tr("Patient data queried"), 1000);
    }

    setEnabled(true);
    updateUiState();
    deleteSenderMlClient(sender());
}

void QueryPage::onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData)
{
    if (error)
    {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("Error while loading possible matches: %1").arg(error.message),
                    QMessageBox::Ok,
                    QMessageBox::Ok);

        mainWindow_->showStatusMessage(tr("Failed to load possible matches"), 5000);
    }
    else
    {
        const auto fieldNames = mainWindow_->endpointSelector()->currentFieldList();

        QList<QStringList> possibleMatchesModelData;

        QStringList modelHeaderRow;
        modelHeaderRow << MlClient::ID_TYPE;
        for (const auto& field : fieldNames)
        {
            modelHeaderRow << field;
        }
        possibleMatchesModelData << modelHeaderRow;

        for (const auto& patientRecord : patientData)
        {
            QStringList modelPatientRecord;
            modelPatientRecord << patientRecord[MlClient::ID_TYPE];
            for (const auto& field : fieldNames)
            {
                modelPatientRecord << patientRecord[field];
            }
            possibleMatchesModelData << modelPatientRecord;
        }

        possibleMatchesModel_->setFirstRowHeader(true);
        possibleMatchesModel_->setModelData(possibleMatchesModelData, false);

        ui->possibleMatches->setEnabled(true);

        mainWindow_->showStatusMessage(tr("Possible matches loaded"), 1000);
    }

    deleteSenderMlClient(sender());
}

void QueryPage::onPossibleMatchesDoubleClicked(const QModelIndex& index)
{
    int pidColumn = -1;
    for (int i = 0; i < possibleMatchesModel_->columnCount(); ++i)
    {
        if (possibleMatchesModel_->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == MlClient::ID_TYPE)
        {
            pidColumn = i;
            break;
        }
    }

    if (pidColumn == -1)
        return;

    const auto pidIndex = possibleMatchesModel_->index(index.row(), pidColumn);
    const auto pid = possibleMatchesModel_->data(pidIndex, Qt::DisplayRole);

    mainWindow_->openPage(MainWindow::Page::Editor, pid);
}
