// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "LoaderPage.h"
#include "ui_LoaderPage.h"

#include "Application.h"
#include "Configuration.h"
#include "CsvReader.h"
#include "DataModel.h"
#include "EndpointConfigModel.h"
#include "MainWindow.h"
#include "PasswordStore.h"
#include "Tools.h"
#include <QDebug>
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

LoaderPage::LoaderPage(QWidget* parent) :
    QWidget{parent},
    mainWindow_{nullptr},
    ui{new Ui::LoaderPage{}},
    inputData_{new DataModel{this}},
    outputData_{new DataModel{this}},
    endpointConfigMapper_{new QDataWidgetMapper{this}}
{
    ui->setupUi(this);
    setup();
}

LoaderPage::~LoaderPage()
{
    saveWidgetState();
    delete ui;
}

void LoaderPage::setup()
{
    ui->inputTable->setModel(inputData_);
    ui->outputTable->setModel(outputData_);

    ui->endpointSelector->setModel(app()->endpointConfigModel());
    ui->endpointSelector->setModelColumn(toInt(EndpointConfig::Field::Name));
    ui->endpointSelector->setCurrentIndex(-1);

    endpointConfigMapper_->setModel(app()->endpointConfigModel());
    endpointConfigMapper_->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    endpointConfigMapper_->addMapping(ui->baseURL, toInt(EndpointConfig::Field::BaseURL));
    endpointConfigMapper_->addMapping(ui->fields, toInt(EndpointConfig::Field::Fields), "items");

    int lineHeight = ui->inputTable->fontMetrics().lineSpacing() + 5;
    ui->inputTable->verticalHeader()->setDefaultSectionSize(lineHeight); // ???

    connect(ui->executeBtn, &QAbstractButton::clicked, this, &LoaderPage::onExecuteButtonClicked);
    connect(ui->loadBtn, &QAbstractButton::clicked, this, &LoaderPage::onLoadButtonClicked);
    connect(ui->saveBtn, &QAbstractButton::clicked, this, &LoaderPage::onSaveButtonClicked);

    connect(this, &LoaderPage::inputLoadingDone, this, &LoaderPage::onInputLoadingDone);
    connect(this, &LoaderPage::outputSavingDone, this, &LoaderPage::onOutputSavingDone);

    connect(inputData_, &DataModel::firstRowHeaderChanged, ui->firstRowIsHeader, &QCheckBox::setChecked);
    connect(ui->firstRowIsHeader, &QCheckBox::stateChanged, inputData_, &DataModel::setFirstRowHeader);

    connect(inputData_, &DataModel::modelReset, this, &LoaderPage::onInputDataChanged);

    connect(ui->endpointSelector, qOverload<int>(&QComboBox::currentIndexChanged), this, &LoaderPage::onEnpointSelectorChanged);

    connect(app()->passwordStore(), &PasswordStore::passwordLoaded, this, &LoaderPage::onPasswordLoaded);
    connect(ui->saveApiKey, &QCheckBox::stateChanged, this, &LoaderPage::onSaveApiKeyChanged);

    connect(ui->pidColumnSelector, qOverload<int>(&QComboBox::currentIndexChanged), this, &LoaderPage::onPidColumSelectorChanged);

    updateUiState();

    loadWidgetState();
}

void LoaderPage::loadWidgetState()
{
    QSettings s;

    ui->endpointSelector->setCurrentIndex(indexClamp(s.value("Window/LoaderPage/SelectedEndpoint").toInt(),
                                                                  app()->endpointConfigModel()->rowCount() - 1));
    ui->splitter->restoreState(s.value("Window/LoaderPage/Splitter").toByteArray());
}

void LoaderPage::saveWidgetState()
{
    QSettings s;

    s.setValue("Window/LoaderPage/SelectedEndpoint", ui->endpointSelector->currentIndex());
    s.setValue("Window/LoaderPage/Splitter", ui->splitter->saveState());
}

void LoaderPage::updateUiState()
{
    auto hasInput = inputData_->hasData();
    auto hasOutput = outputData_->hasData();
    auto configComplete =
            // TODO check if ml config is ok
            ui->pidColumnSelector->currentIndex() != -1
            ;

    ui->executeBtn->setEnabled(hasInput && configComplete);
    ui->outputArea->setEnabled(hasInput);
    ui->saveBtn->setEnabled(hasOutput);
}

void LoaderPage::onInputLoadingDone(bool result)
{
    Q_ASSERT(mainWindow_);

    if (result)
        mainWindow_->showStatusMessage(tr("PID file loaded successful"), 1000);
    else
        mainWindow_->showStatusMessage(tr("Failed to load PID file"), 2000);

    outputData_->setModelData({}, false);

    updateUiState();
}

void LoaderPage::onOutputSavingDone(bool result)
{
    Q_ASSERT(mainWindow_);

    if (result)
        mainWindow_->showStatusMessage(tr("Patient data file saved successful"), 1000);
    else
        mainWindow_->showStatusMessage(tr("Failed to save patient data file"), 2000);
}

void LoaderPage::onInputDataChanged()
{
    QStringList columnNames;
    for (int i = 0; i < inputData_->columnCount(); ++i)
    {
        columnNames << inputData_->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
    }

    while (ui->pidColumnSelector->count() > 0)
    {
        ui->pidColumnSelector->removeItem(0);
    }
    ui->pidColumnSelector->addItems(columnNames);
    ui->pidColumnSelector->setCurrentIndex(inputData_->detectedPidColumn());
}

void LoaderPage::onEnpointSelectorChanged(int index)
{
    endpointConfigMapper_->setCurrentIndex(index);

    if (index == -1)
        return;

    ui->fields->selectAll();

    app()->passwordStore()->loadPassword(currentEndpointUuid());
}

void LoaderPage::onPasswordLoaded(bool result, const QUuid& uuid, const QString& password)
{
    if (uuid != currentEndpointUuid())
        return;

    ui->apiKey->setText(password);
}

void LoaderPage::onSaveApiKeyChanged(bool state)
{
    if (!state)
        app()->passwordStore()->removePassword(currentEndpointUuid());
    else
        ui->apiKey->setModified(true);
}

void LoaderPage::onPidColumSelectorChanged(int index)
{
    updateUiState();
}

void LoaderPage::onPatientDataLoaded(bool result, const MlClient::PatientData& patientData)
{
    qCDebug(MLR_LOG_CAT) << "Loader Execution: Fetching took" << executionTimer_.elapsed() << "ms";
    executionTimer_.restart();

    setEnabled(true);

    auto mlClient = qobject_cast<MlClient*>(sender());
    Q_ASSERT(mlClient);

    if (result)
    {
        mergePatientData(patientData);

        mainWindow_->showStatusMessage(tr("Patient data loaded"), 1000);
    }
    else
    {
        mainWindow_->showStatusMessage(tr("Failed to load Patient data"), 2000);
    }

    qCDebug(MLR_LOG_CAT) << "Loader Execution: Merging took" << executionTimer_.elapsed() << "ms";
    executionTimer_.restart();
    qCDebug(MLR_LOG_CAT) << "Loader Execution: Stopped";

    updateUiState();

    mlClient->deleteLater();
}

void LoaderPage::onExecuteButtonClicked()
{
    if (ui->apiKey->isModified())
    {
        if (ui->saveApiKey->isChecked())
            app()->passwordStore()->savePassword(currentEndpointUuid(), ui->apiKey->text());
        ui->apiKey->setModified(false);
    }

    executionTimer_.start();
    qCDebug(MLR_LOG_CAT) << "Loader Execution: Started";

    auto fieldList = makeFieldList();
    if (fieldList.isEmpty())
    {
        mainWindow_->showStatusMessage(tr("No fields selected"), 1000);
        return;
    }

    setEnabled(false);
    mainWindow_->showStatusMessage(tr("Loading patient data ..."));

    const auto model = app()->endpointConfigModel();
    int currentEndpointIndex = ui->endpointSelector->currentIndex();

    const auto baseUrl = model->data(
                model->index(currentEndpointIndex, toInt(EndpointConfig::Field::BaseURL)),
                Qt::DisplayRole).toString();
    const auto apiVersion = model->data(
                model->index(currentEndpointIndex, toInt(EndpointConfig::Field::ApiVersion)),
                Qt::DisplayRole).toString();

    auto mlClient = new MlClient{
            baseUrl,
            QVersionNumber::fromString(apiVersion),
            ui->apiKey->text()};

    connect(mlClient, &MlClient::patientDataLoaded, this, &LoaderPage::onPatientDataLoaded);

    qCDebug(MLR_LOG_CAT) << "Loader Execution: Setup took" << executionTimer_.elapsed() << "ms";
    executionTimer_.restart();

    mlClient->loadPatientData(makePidList(), fieldList);
}

void LoaderPage::onLoadButtonClicked()
{
    auto cfg = app()->configuration();

    auto fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open PID file (csv list)"),
                cfg->stringValue(Configuration::Key::LastAccessedDirectory),
                tr("CSV Files (*.csv);;All Files (*.*)")
                );

    if (fileName.isEmpty())
        return;

    QFileInfo fi{fileName};
    cfg->setValue(Configuration::Key::LastAccessedDirectory, fi.absolutePath());

    readInput(fileName);
}

void LoaderPage::onSaveButtonClicked()
{
    auto cfg = app()->configuration();

    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save to patient data file (csv list)"),
                cfg->stringValue(Configuration::Key::LastAccessedDirectory),
                tr("CSV Files (*.csv);;All Files (*.*)")
                );

    if (fileName.isEmpty())
        return;

    QFileInfo fi{fileName};
    cfg->setValue(Configuration::Key::LastAccessedDirectory, fi.absolutePath());

    writeOutput(fileName);
}

void LoaderPage::readInput(const QString &fileName)
{
    Q_ASSERT(mainWindow_);
    mainWindow_->showStatusMessage(tr("Loading PID file ..."));

    app()->runJob([this, fileName]()
    {
        bool result{};

        QFile input{fileName};
        if (input.open(QFile::ReadOnly))
        {
            CsvReader csvReader{};
            result = csvReader.read(input, inputData_);
        }

        emit inputLoadingDone(result);
    });
}

void LoaderPage::writeOutput(const QString &fileName)
{
    Q_ASSERT(mainWindow_);
    mainWindow_->showStatusMessage(tr("Saving patient data file ..."));

    app()->runJob([this, fileName]()
    {
        bool result{};

        QFile output{fileName};
        if (output.open(QFile::WriteOnly | QFile::Truncate))
        {
            CsvReader csvReader{};
            result = csvReader.write(output, ui->exportHeaders->isChecked(), outputData_);
        }

        emit outputSavingDone(result);
    });
}

QUuid LoaderPage::currentEndpointUuid()
{
    auto index = ui->endpointSelector->currentIndex();
    if (index == -1)
        return {};

    auto* model = app()->endpointConfigModel();
    auto uuidData = model->data(model->index(index, toInt(EndpointConfig::Field::Uuid)),
                            Qt::DisplayRole);
    return uuidData.toUuid();
}

QStringList LoaderPage::makePidList()
{
    const int pidColumn = ui->pidColumnSelector->currentIndex();

    QStringList pids;
    for (int i = 0; i < inputData_->rowCount(); ++i)
    {
        pids << inputData_->data(inputData_->index(i, pidColumn), Qt::DisplayRole).toString();
    }

    return pids;
}

QStringList LoaderPage::makeFieldList()
{
    auto fieldsSelection = ui->fields->selectionModel()->selection().indexes();

    QStringList fields;
    for (const auto& idx : fieldsSelection)
    {
        fields << idx.data(Qt::DisplayRole).toString();
    }

    return fields;
}

void LoaderPage::mergePatientData(const MlClient::PatientData& patientData)
{
    // make index for patient data
    QHash<QString, int> pidIndex;
    for (int i = 0; i < patientData.count(); ++i)
    {
        const auto pid = patientData[i]["pid"_l1];
        pidIndex.insert(pid, i);
    }

    // get current state
    const int pidColumn = ui->pidColumnSelector->currentIndex();
    const auto fields = makeFieldList();

    QList<QStringList> modelData;

    // header header row
    QStringList headerRowData;
    for (int col = 0; col < inputData_->columnCount(); ++col)
    {
        headerRowData << inputData_->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
    }
    for (const auto& field : fields)
    {
        headerRowData << field;
    }
    modelData << headerRowData;

    // merge and add patient records
    for (int row = 0; row < inputData_->rowCount(); ++row)
    {
        const auto rowPid = inputData_->data(inputData_->index(row, pidColumn), Qt::DisplayRole).toString();

        QStringList rowData;

        for (int col = 0; col < inputData_->columnCount(); ++col)
        {
            rowData << inputData_->data(inputData_->index(row, col), Qt::DisplayRole).toString();
        }

        const auto patientDataIndex = pidIndex.find(rowPid);
        if (patientDataIndex == pidIndex.end())
        {
            qCWarning(MLR_LOG_CAT) << "PID" << rowPid << "not found in ML result";

            for (int i = 0; i < fields.count(); ++i)
                rowData << QString{};

            continue;
        }

        const auto& patientRecord = patientData[*patientDataIndex];
        for (const auto& field : fields)
        {
            rowData << patientRecord[field];
        }

        modelData << rowData;
    }

    outputData_->setFirstRowHeader(true);
    outputData_->setModelData(modelData, false);
}
