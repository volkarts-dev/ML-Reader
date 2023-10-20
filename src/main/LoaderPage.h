// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include "MlClient.h"
#include <QElapsedTimer>
#include <QWidget>

class DataModel;
class MainWindow;
class QDataWidgetMapper;

namespace Ui {
class LoaderPage;
}

class LoaderPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoaderPage(QWidget* parent = {});
    ~LoaderPage() override;

    void setMainWindow(MainWindow* mainWindow) { mainWindow_ = mainWindow; } // TODO us an interface

signals:
    void inputLoadingDone(bool result);
    void outputSavingDone(bool result);

private slots:
    void onExecuteButtonClicked();
    void onLoadButtonClicked();
    void onSaveButtonClicked();
    void onInputLoadingDone(bool result);
    void onOutputSavingDone(bool result);
    void onInputDataChanged();
    void onEnpointSelectorChanged(int index);
    void onPasswordLoaded(bool result, const QUuid& uuid, const QString& password);
    void onSaveApiKeyChanged(bool state);
    void onPidColumSelectorChanged(int index);
    void onPatientDataLoaded(bool result, const MlClient::PatientData& patientData);

private:
    void setup();
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void readInput(const QString& fileName);
    void writeOutput(const QString& fileName);
    QUuid currentEndpointUuid();
    QStringList makePidList();
    QStringList makeFieldList();
    void mergePatientData(const MlClient::PatientData& patientData);

private:
    MainWindow* mainWindow_;
    Ui::LoaderPage* ui;
    DataModel* inputData_{};
    DataModel* outputData_{};
    QDataWidgetMapper* endpointConfigMapper_;
    QElapsedTimer executionTimer_;

    Q_DISABLE_COPY_MOVE(LoaderPage)
};
