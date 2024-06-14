// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EditorPage.h"
#include "ui_EditorPage.h"

#include "Application.h"
#include "EndpointConfig.h"
#include "EndpointConfigModel.h"
#include "EndpointSelector.h"
#include "MainWindow.h"
#include "MlClientTools.h"
#include "Tools.h"
#include <QMessageBox>

EditorPage::EditorPage(QWidget *parent) :
    QWidget{parent},
    ui{new Ui::EditorPage{}}
{
    ui->setupUi(this);
}

EditorPage::~EditorPage()
{
    saveWidgetState();

    delete ui;
}

void EditorPage::initialize(MainWindow* mainWindow)
{
    mainWindow_ = mainWindow;

    ui->abortBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));

    connect(mainWindow_, &MainWindow::endpointConfigChanged, this, &EditorPage::onEndpointConfigChanged);
    connect(mainWindow_, &MainWindow::selectedEndpointChanged, this, &EditorPage::onSelectedEndpointChanged);

    connect(ui->loadIDataBtn, &QAbstractButton::clicked, this, &EditorPage::onLoadIDataBtnClicked);
    connect(ui->saveBtn, &QAbstractButton::clicked, this, &EditorPage::onSaveBtnClicked);
    connect(ui->abortBtn, &QAbstractButton::clicked, this, &EditorPage::onAbortBtnClicked);

    loadWidgetState();

    updateUiState();
}

void EditorPage::loadWidgetState()
{
}

void EditorPage::saveWidgetState()
{
}

void EditorPage::updateUiState()
{
    bool endpointSelected = mainWindow_->endpointSelector()->selectedEndpoint() != -1;
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

    auto mlClient = createMlClient(mainWindow_->endpointSelector()->selectedEndpoint(),
                                   mainWindow_->endpointSelector()->currentApiKey(),
                                   mainWindow_, &MainWindow::logMessage);
    mlClientLoadPatientData(mlClient, pidList, mainWindow_->endpointSelector()->currentFieldList(),
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

void EditorPage::onLoadIDataBtnClicked()
{
    startEditing();
}

void EditorPage::onSaveBtnClicked()
{
    Q_ASSERT(!loadedPatientPid_.isEmpty());

    auto mlClient = createMlClient(mainWindow_->endpointSelector()->selectedEndpoint(),
                                   mainWindow_->endpointSelector()->currentApiKey(),
                                   mainWindow_, &MainWindow::logMessage);
    mlClientEditPatientData(mlClient, loadedPatientPid_,
                            ui->patientDataForm->extractFormData(DynamicForm::Filter::Modified),
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

        mainWindow_->showStatusMessage(tr("Failed to load patient data"), 5000);
    }
    else
    {
        ui->patientDataForm->fillFormData(patientData[0]);
        loadedPatientPid_ = ui->searchPid->text();

        mainWindow_->showStatusMessage(tr("Patient data loaded"), 1000);
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

        mainWindow_->showStatusMessage(tr("Failed to edit patient data"), 5000);
    }
    else
    {
        mainWindow_->showStatusMessage(tr("Patient data edited"), 1000);
    }

    setEnabled(true);
    updateUiState();
    deleteSenderMlClient(sender());
}

void EditorPage::onEndpointConfigChanged()
{
    reloadDynamicForm(mainWindow_->endpointSelector()->selectedEndpoint());
}

void EditorPage::onSelectedEndpointChanged(int index)
{
    reloadDynamicForm(index);

    updateUiState();
}
