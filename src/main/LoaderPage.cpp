// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "LoaderPage.h"
#include "ui_LoaderPage.h"

#include "Application.h"
#include "CsvReader.h"
#include "DataModel.h"
#include "EndpointConfigModel.h"
#include "EndpointSelector.h"
#include "MainWindow.h"
#include "MlClientTools.h"
#include "Tools.h"
#include "UserSettings.h"
#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>

namespace {

const auto MimeTypeTextPlain = QStringLiteral("text/plain");
const auto CfgPageSplitter= QStringLiteral("Window/LoaderPage/Splitter");

} // namespace

LoaderPage::LoaderPage(QWidget* parent) :
    QWidget{parent},
    ui{new Ui::LoaderPage{}},
    inputData_{new DataModel{this}},
    outputData_{new DataModel{this}}
{
    ui->setupUi(this);
}

LoaderPage::~LoaderPage()
{
    saveWidgetState();

    delete ui;
}

void LoaderPage::initialize(MainWindow* mainWindow)
{
    mainWindow_ = mainWindow;

    ui->inputTable->setModel(inputData_);
    ui->outputTable->setModel(outputData_);

    int lineHeight = ui->inputTable->fontMetrics().lineSpacing() + 5;
    ui->inputTable->verticalHeader()->setDefaultSectionSize(lineHeight); // ???

    connect(mainWindow_, &MainWindow::endpointConfigChanged, this, &LoaderPage::onEndpointConfigChanged);
    connect(mainWindow_, &MainWindow::selectedEndpointChanged, this, &LoaderPage::onSelectedEndpointChanged);

    connect(ui->executeBtn, &QAbstractButton::clicked, this, &LoaderPage::onExecuteButtonClicked);
    connect(ui->pasteBtn, &QAbstractButton::clicked, this, &LoaderPage::onPasteButtonClicked);
    connect(ui->loadBtn, &QAbstractButton::clicked, this, &LoaderPage::onLoadButtonClicked);
    connect(ui->saveBtn, &QAbstractButton::clicked, this, &LoaderPage::onSaveButtonClicked);
    connect(ui->clearInputBtn, &QAbstractButton::clicked, this, &LoaderPage::onClearInputButtonClicked);

    connect(this, &LoaderPage::inputLoadingDone, this, &LoaderPage::onInputLoadingDone);
    connect(this, &LoaderPage::outputSavingDone, this, &LoaderPage::onOutputSavingDone);

    connect(inputData_, &DataModel::firstRowHeaderChanged, ui->firstRowIsHeader, &QCheckBox::setChecked);
    connect(ui->firstRowIsHeader, &QCheckBox::stateChanged, inputData_, &DataModel::setFirstRowHeader);
    ui->firstRowIsHeader->setChecked(inputData_->firstRowHeader());

    connect(inputData_, &DataModel::modelReset, this, &LoaderPage::onInputDataChanged);

    connect(ui->pidColumnSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &LoaderPage::onPidColumSelectorChanged);

    ui->inputArea->setAllowedDropActions(Qt::CopyAction | Qt::MoveAction);
    ui->inputArea->setAllowedMimeTypes({MimeTypeTextPlain});
    connect(ui->inputArea, &InputDropZone::dataDropped, this, &LoaderPage::onInputDataDropped);

    updateUiState();

    loadWidgetState();

    // disable for now
    ui->clearInputBtn->setVisible(false);
}

void LoaderPage::loadWidgetState()
{
    UserSettings s;

    restoreSplitterState(ui->splitter, s.value(CfgPageSplitter).toByteArray());
}

void LoaderPage::saveWidgetState()
{
    UserSettings s;

    s.setValue(CfgPageSplitter, saveSplitterState(ui->splitter));
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
        mainWindow_->showStatusMessage(tr("Failed to load PID file"), 5000);

    outputData_->setModelData({}, false);

    updateUiState();
}

void LoaderPage::onOutputSavingDone(bool result)
{
    Q_ASSERT(mainWindow_);

    if (result)
        mainWindow_->showStatusMessage(tr("Patient data file saved successful"), 1000);
    else
        mainWindow_->showStatusMessage(tr("Failed to save patient data file"), 5000);
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

void LoaderPage::onPidColumSelectorChanged(int index)
{
    Q_UNUSED(index)

    updateUiState();
}

void LoaderPage::onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData)
{
    qCDebug(MLR_LOG_CAT) << "Loader Execution: Fetching took" << executionTimer_.elapsed() << "ms";
    executionTimer_.restart();

    if (error)
    {
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("Error while loading patient data: %1").arg(error.message),
                    QMessageBox::Ok,
                    QMessageBox::Ok);

        mainWindow_->showStatusMessage(tr("Failed to load Patient data"), 5000);
    }
    else
    {
        mergePatientData(patientData);

        qCDebug(MLR_LOG_CAT) << "Loader Execution: Merging took" << executionTimer_.elapsed() << "ms";
        executionTimer_.restart();
        qCDebug(MLR_LOG_CAT) << "Loader Execution: Stopped";

        mainWindow_->showStatusMessage(tr("Patient data loaded"), 1000);
    }

    setEnabled(true);
    updateUiState();
    deleteSenderMlClient(sender());
}

void LoaderPage::onExecuteButtonClicked()
{
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

    qCDebug(MLR_LOG_CAT) << "Loader Execution: Setup took" << executionTimer_.elapsed() << "ms";
    executionTimer_.restart();

    auto mlClient = createMlClient(mainWindow_->endpointSelector()->selectedEndpoint(),
                                   mainWindow_->endpointSelector()->currentApiKey(),
                                   mainWindow_, &MainWindow::logMessage);
    mlClientLoadPatientData(mlClient, makePidList(), fieldList, this, &LoaderPage::onPatientDataLoadingDone);
}

void LoaderPage::onPasteButtonClicked()
{
    readInputFromClipboard();
}

void LoaderPage::onLoadButtonClicked()
{
    UserSettings s;

    auto fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open PID file (csv list)"),
                s.stringValue(CfgLastAccessedDirectory),
                tr("CSV Files (*.csv);;All Files (*.*)")
                );

    if (fileName.isEmpty())
        return;

    QFileInfo fi{fileName};
    s.setValue(CfgLastAccessedDirectory, fi.absolutePath());

    readInputFromFile(fileName);
}

void LoaderPage::onClearInputButtonClicked()
{
    // disabled for now
}

void LoaderPage::onSaveButtonClicked()
{
    UserSettings s;

    auto fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save to patient data file (csv list)"),
                s.stringValue(CfgLastAccessedDirectory),
                tr("CSV Files (*.csv);;All Files (*.*)")
                );

    if (fileName.isEmpty())
        return;

    QFileInfo fi{fileName};
    s.setValue(CfgLastAccessedDirectory, fi.absolutePath());

    writeOutput(fileName);
}

void LoaderPage::onEndpointConfigChanged()
{
    reloadFieldList(mainWindow_->endpointSelector()->selectedEndpoint());
}

void LoaderPage::onSelectedEndpointChanged(int index)
{
    reloadFieldList(index);
}

void LoaderPage::onInputDataDropped(const QMimeData* mimeData)
{
    Q_ASSERT(mainWindow_);
    mainWindow_->showStatusMessage(tr("Loading PID data ..."));

    if (!mimeData->hasText())
        return;

    auto rawData = mimeData->text();

    app()->runJob([this, rawData]()
    {
        auto bytes = rawData.toUtf8();
        QBuffer input{&bytes};

        bool result = readInput(input);

        emit inputLoadingDone(result);
    });
}

void LoaderPage::readInputFromFile(const QString& fileName)
{
    Q_ASSERT(mainWindow_);
    mainWindow_->showStatusMessage(tr("Loading PID file ..."));

    app()->runJob([this, fileName]()
    {
        QFile input{fileName};

        bool result = readInput(input);

        emit inputLoadingDone(result);
    });
}

void LoaderPage::readInputFromClipboard()
{
    Q_ASSERT(mainWindow_);
    mainWindow_->showStatusMessage(tr("Loading PID data ..."));

    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if (!mimeData->hasText())
        return;

    auto rawData = mimeData->text();

    app()->runJob([this, rawData]()
    {
        auto bytes = rawData.toUtf8();
        QBuffer input{&bytes};

        bool result = readInput(input);

        emit inputLoadingDone(result);
    });
}

bool LoaderPage::readInput(QIODevice& input)
{
    if (input.open(QBuffer::ReadOnly))
    {
        CsvReader csvReader{};
        return csvReader.read(input, inputData_);
    }

    return false;
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
    if (fieldsSelection.empty())
    {
        return ui->fields->items();
    }

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

void LoaderPage::reloadFieldList(int endpointIndex)
{
    if (endpointIndex == -1)
    {
        ui->fields->setItems({});
        return;
    }

    const auto model = app()->endpointConfigModel();
    const auto modelIndex = model->index(endpointIndex, toInt(EndpointConfig::Field::Fields));
    ui->fields->setItems(model->data(modelIndex, Qt::DisplayRole).toStringList());

    ui->fields->selectAll();
}
