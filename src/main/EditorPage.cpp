// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

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

    connect(ui->endpointSelector, &EndpointSelector::selectedEnpointChanged, this, &EditorPage::onSelectedEnpointChanged);

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
                            this, &EditorPage::onPatientDataLoaded, &EditorPage::onPatientDataLoadingFailed);
}

void EditorPage::onSelectedEnpointChanged(int index)
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

void EditorPage::onLoadIDataBtnClicked()
{
    startEditing();
}

void EditorPage::onSaveBtnClicked()
{
    Q_ASSERT(!loadedPatientPid_.isEmpty());

    auto mlClient = createMlClient(ui->endpointSelector->selectedEndpoint(), ui->endpointSelector->currentApiKey());
    mlClientEditPatientData(mlClient, loadedPatientPid_, ui->patientDataForm->extractFormData(DynamicForm::Filter::Modified),
                            this, &EditorPage::onPatientDataEdited, &EditorPage::onPatientDataEditingFailed);
}

void EditorPage::onAbortBtnClicked()
{
    loadedPatientPid_ = QString{};
    updateUiState();
}

void EditorPage::onPatientDataLoadingFailed(const QString& error)
{
    setEnabled(true);

    mainInterface_->showStatusMessage(tr("Failed to load patient data"), 2000);

    QMessageBox::warning(
                this,
                tr("Error"),
                tr("Error while loading patient data: %1").arg(error),
                QMessageBox::Ok,
                QMessageBox::Ok);

    updateUiState();

    deleteSenderMlClient(sender());
}

void EditorPage::onPatientDataLoaded(const MlClient::PatientData& patientData)
{
    setEnabled(true);

    ui->patientDataForm->fillFormData(patientData[0]);

    mainInterface_->showStatusMessage(tr("Patient data loaded"), 1000);

    loadedPatientPid_ = ui->searchPid->text();
    updateUiState();

    deleteSenderMlClient(sender());
}

void EditorPage::onPatientDataEditingFailed(const QString& error)
{
    setEnabled(true);

    mainInterface_->showStatusMessage(tr("Failed to edit patient data"), 2000);

    QMessageBox::warning(
                this,
                tr("Error"),
                tr("Error while edit patient data: %1").arg(error),
                QMessageBox::Ok,
                QMessageBox::Ok);

    updateUiState();

    deleteSenderMlClient(sender());
}

void EditorPage::onPatientDataEdited()
{
    setEnabled(true);

    mainInterface_->showStatusMessage(tr("Patient data edited"), 1000);

    deleteSenderMlClient(sender());
}
