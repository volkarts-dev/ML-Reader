// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "QueryPage.h"
#include "ui_QueryPage.h"

#include "Application.h"
#include "DataModel.h"
#include "EndpointConfig.h"
#include "EndpointConfigModel.h"
#include "MainInterface.h"
#include "MlClientTools.h"
#include "Tools.h"
#include <QClipboard>
#include <QMessageBox>
#include <QSettings>

QueryPage::QueryPage(QWidget *parent) :
    QWidget{parent},
    ui{new Ui::QueryPage{}},
    possibleMatchesModel_{new DataModel{this}}
{
    ui->setupUi(this);
    setup();
}

QueryPage::~QueryPage()
{
    saveWidgetState();

    delete ui;
}

void QueryPage::setup()
{
    ui->queryResultPane->setVisible(false);
    ui->possibleMatchesPane->setVisible(false);

    ui->possibleMatches->setModel(possibleMatchesModel_);

    connect(ui->endpointSelector, &EndpointSelector::selectedEnpointChanged, this, &QueryPage::onSelectedEnpointChanged);

    connect(ui->executeBtn, &QAbstractButton::clicked, this, &QueryPage::onExecuteButtonClicked);
    connect(ui->editPatientBtn, &QAbstractButton::clicked, this, &QueryPage::onEditPatientBtnClicked);
    connect(ui->copyPidBtn, &QAbstractButton::clicked, this, &QueryPage::onCopyPidBtnClicked);

    loadWidgetState();
}

void QueryPage::loadWidgetState()
{
    QSettings s;

    ui->endpointSelector->setSelectedEndpoint(
                indexClamp(s.value("Window/QueryPage/SelectedEndpoint").toInt(),
                           app()->endpointConfigModel()->rowCount() - 1));
    ui->splitter->restoreState(s.value("Window/QueryPage/Splitter").toByteArray());
}

void QueryPage::saveWidgetState()
{
    QSettings s;

    s.setValue("Window/QueryPage/SelectedEndpoint", ui->endpointSelector->selectedEndpoint());
    s.setValue("Window/QueryPage/Splitter", ui->splitter->saveState());
}

void QueryPage::updateUiState()
{
    bool endpointSelected = ui->endpointSelector->selectedEndpoint() != -1;

    ui->executeBtn->setEnabled(endpointSelected);
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

void QueryPage::onSelectedEnpointChanged(int index)
{
    QList<DynamicForm::Field> dynamicFields;

    if (index != -1)
    {
        const auto model = app()->endpointConfigModel();
        const auto modelIndex = model->index(index, toInt(EndpointConfig::Field::Fields));
        const auto fields = model->data(modelIndex, Qt::DisplayRole).toStringList();

        for (const auto& field : fields)
        {
            dynamicFields << DynamicForm::Field{field};
        }
    }

    ui->patientDataForm->reset(dynamicFields);

    updateUiState();
}

void QueryPage::onExecuteButtonClicked()
{
    setEnabled(false);

    auto mlClient = createMlClient(ui->endpointSelector->selectedEndpoint(), ui->endpointSelector->currentApiKey());
    mlClientQueryPatientData(mlClient, ui->patientDataForm->extractFormData(),
                             this, &QueryPage::onPatientDataQueried, &QueryPage::onPatientDataQueringFailed);
}

void QueryPage::onEditPatientBtnClicked()
{
    mainInterface_->openPage(MainInterface::Page::Editor, ui->patientPid->text());
}

void QueryPage::onCopyPidBtnClicked()
{
    QGuiApplication::clipboard()->setText(ui->patientPid->text());
    mainInterface_->showStatusMessage(tr("Copied PID to Clipboard"), 1000);
}

void QueryPage::onPatientDataQueringFailed(const QString& error)
{
    setEnabled(true);

    mainInterface_->showStatusMessage(tr("Failed to query Patient data"), 2000);

    QMessageBox::warning(
                this,
                tr("Error"),
                tr("Error while quering patient data: %1").arg(error),
                QMessageBox::Ok,
                QMessageBox::Ok);

    deleteSenderMlClient(sender());
}

void QueryPage::onPatientDataQueried(const MlClient::QueryResult& result)
{
    setEnabled(true);

    mainInterface_->showStatusMessage(tr("Patient data queried"), 1000);

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

        auto mlClient = createMlClient(ui->endpointSelector->selectedEndpoint(), ui->endpointSelector->currentApiKey());
        mlClientLoadPatientData(mlClient, result.possibleMatchPids, ui->endpointSelector->currentFieldList(),
                                 this, &QueryPage::onPatientDataLoaded, &QueryPage::onPatientDataLoadingFailed);
    }

    updateUiState();

    deleteSenderMlClient(sender());
}

void QueryPage::onPatientDataLoadingFailed(const QString& error)
{
    mainInterface_->showStatusMessage(tr("Failed to load possible matches"), 2000);

    QMessageBox::warning(
                this,
                tr("Error"),
                tr("Error while loading possible matches: %1").arg(error),
                QMessageBox::Ok,
                QMessageBox::Ok);

    deleteSenderMlClient(sender());
}

void QueryPage::onPatientDataLoaded(const MlClient::PatientData& patientData)
{
    const auto fieldNames = ui->endpointSelector->currentFieldList();

    QList<QStringList> possibleMatchesModelData;

    QStringList modelHeaderRow;
    for (const auto& field : fieldNames)
    {
        modelHeaderRow << field;
    }
    possibleMatchesModelData << modelHeaderRow;

    for (const auto& patientRecord : patientData)
    {
        QStringList modelPatientRecord;
        for (const auto& field : fieldNames)
        {
            modelPatientRecord << patientRecord[field];
        }
        possibleMatchesModelData << modelPatientRecord;
    }

    possibleMatchesModel_->setFirstRowHeader(true);
    possibleMatchesModel_->setModelData(possibleMatchesModelData, false);

    ui->possibleMatches->setEnabled(true);

    mainInterface_->showStatusMessage(tr("Possible matches loaded"), 1000);

    deleteSenderMlClient(sender());
}
