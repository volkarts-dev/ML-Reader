// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EditorPage.h"
#include "ui_EditorPage.h"

#include "Application.h"
#include "EndpointConfig.h"
#include "EndpointConfigModel.h"
#include "MainInterface.h"
#include "MlClientTools.h"
#include "Tools.h"
#include <QMessageBox>
#include <QSettings>

EditorPage::EditorPage(QWidget *parent) :
    QWidget{parent},
    ui{new Ui::EditorPage{}}
{
    ui->setupUi(this);
    setup();
}

EditorPage::~EditorPage()
{
    saveWidgetState();

    delete ui;
}

void EditorPage::setup()
{
    ui->abortBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));

    connect(ui->endpointSelector, &EndpointSelector::selectedEndpointChanged, this, &EditorPage::onSelectedEndpointChanged);

    connect(ui->loadIDataBtn, &QAbstractButton::clicked, this, &EditorPage::onLoadIDataBtnClicked);
    connect(ui->saveBtn, &QAbstractButton::clicked, this, &EditorPage::onSaveBtnClicked);
    connect(ui->abortBtn, &QAbstractButton::clicked, this, &EditorPage::onAbortBtnClicked);

    loadWidgetState();

    updateUiState();
}

void EditorPage::loadWidgetState()
{
    QSettings s;

    ui->endpointSelector->setSelectedEndpoint(
                indexClamp(s.value("Window/EditPage/SelectedEndpoint").toInt(),
                           app()->endpointConfigModel()->rowCount() - 1));
}

void EditorPage::saveWidgetState()
{
    QSettings s;

    s.setValue("Window/EditPage/SelectedEndpoint", ui->endpointSelector->selectedEndpoint());
}

void EditorPage::updateUiState()
{
    bool endpointSelected = ui->endpointSelector->selectedEndpoint() != -1;
    bool dataLoaded = !loadedPatientPid_.isEmpty();

    ui->searchPid->setEnabled(endpointSelected && !dataLoaded);
    ui->loadIDataBtn->setEnabled(endpointSelected && !dataLoaded);
    ui->saveBtn->setEnabled(dataLoaded);
    ui->abortBtn->setEnabled(dataLoaded);
    ui->patientDataForm->setEnabled(dataLoaded);
}

void EditorPage::changeEvent(QEvent* event)
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

void EditorPage::startEditing(const QString& pid)
{
    ui->searchPid->setText(pid);
    startEditing();
}

void EditorPage::handleEndpointConfigChanged()
{
    reloadDynamicForm(ui->endpointSelector->selectedEndpoint());
}

void EditorPage::setSelectedEndpoint(int index)
{
    ui->endpointSelector->setSelectedEndpoint(index);
}

void EditorPage::startEditing()
{
    const auto pid = ui->searchPid->text();
    if (pid.isEmpty())
        return;

    setEnabled(false);

    loadedPatientPid_ = QString{};

    QStringList pidList{pid};

    auto mlClient = createMlClient(ui->endpointSelector->selectedEndpoint(), ui->endpointSelector->currentApiKey());
    mlClientLoadPatientData(mlClient, pidList, ui->endpointSelector->currentFieldList(),
                            this, &EditorPage::onPatientDataLoadingDone);
}

void EditorPage::reloadDynamicForm(int endpointIndex)
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

void EditorPage::onSelectedEndpointChanged(int index)
{
    reloadDynamicForm(index);

    updateUiState();

    emit selectedEndpointChanged(index);
}

void EditorPage::onLoadIDataBtnClicked()
{
    startEditing();
}

void EditorPage::onSaveBtnClicked()
{
    Q_ASSERT(!loadedPatientPid_.isEmpty());

    auto mlClient = createMlClient(ui->endpointSelector->selectedEndpoint(), ui->endpointSelector->currentApiKey());
    mlClientEditPatientData(mlClient, loadedPatientPid_, ui->patientDataForm->extractFormData(DynamicForm::Filter::Modified),
                            this, &EditorPage::onPatientDataEditingDone);
}

void EditorPage::onAbortBtnClicked()
{
    loadedPatientPid_ = QString{};
    updateUiState();
}

void EditorPage::onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData)
{
    if (error)
    {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("Error while loading patient data: %1").arg(error.message),
                    QMessageBox::Ok,
                    QMessageBox::Ok);

        mainInterface_->showStatusMessage(tr("Failed to load patient data"), 5000);
    }
    else
    {
        ui->patientDataForm->fillFormData(patientData[0]);
        loadedPatientPid_ = ui->searchPid->text();

        mainInterface_->showStatusMessage(tr("Patient data loaded"), 1000);
    }

    setEnabled(true);
    updateUiState();
    deleteSenderMlClient(sender());
}

void EditorPage::onPatientDataEditingDone(const MlClient::Error& error)
{
    if (error)
    {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("Error while edit patient data: %1").arg(error.message),
                    QMessageBox::Ok,
                    QMessageBox::Ok);

        mainInterface_->showStatusMessage(tr("Failed to edit patient data"), 5000);
    }
    else
    {
        mainInterface_->showStatusMessage(tr("Patient data edited"), 1000);
    }

    setEnabled(true);
    updateUiState();
    deleteSenderMlClient(sender());
}
