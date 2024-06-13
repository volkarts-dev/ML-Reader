// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QElapsedTimer>
#include <QWidget>

class DataModel;
class MainWindow;
class QDataWidgetMapper;
class QMimeData;

namespace Ui {
class LoaderPage;
}

class LoaderPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoaderPage(QWidget* parent = {});
    ~LoaderPage() override;

    void initialize(MainWindow* mainWindow);

signals:
    void inputLoadingDone(bool result);
    void outputSavingDone(bool result);

private slots:
    void onExecuteButtonClicked();
    void onPasteButtonClicked();
    void onLoadButtonClicked();
    void onClearInputButtonClicked();
    void onSaveButtonClicked();
    void onInputLoadingDone(bool result);
    void onOutputSavingDone(bool result);
    void onInputDataChanged();
    void onPidColumSelectorChanged(int index);
    void onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData);
    void onEndpointConfigChanged();
    void onSelectedEndpointChanged(int index);
    void onInputDataDropped(const QMimeData* mimeData);

private:
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void readInputFromFile(const QString& fileName);
    void readInputFromClipboard();
    bool readInput(QIODevice& input);
    void writeOutput(const QString& fileName);
    QStringList makePidList();
    QStringList makeFieldList();
    void mergePatientData(const MlClient::PatientData& patientData);
    void reloadFieldList(int endpointIndex);

private:
    Ui::LoaderPage* ui;
    MainWindow* mainWindow_{};
    DataModel* inputData_{};
    DataModel* outputData_{};
    QElapsedTimer executionTimer_;

    Q_DISABLE_COPY_MOVE(LoaderPage)
};
