// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QElapsedTimer>
#include <QWidget>

class DataModel;
class MainInterface;
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

    void setMainInterface(MainInterface* mainInterface) { mainInterface_ = mainInterface; }

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
    void onSelectedEnpointChanged(int index);
    void onPidColumSelectorChanged(int index);
    void onPatientDataLoadingFailed(const QString& error);
    void onPatientDataLoaded(const MlClient::PatientData& patientData);

private:
    void setup();
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void readInput(const QString& fileName);
    void writeOutput(const QString& fileName);
    QStringList makePidList();
    QStringList makeFieldList();
    void mergePatientData(const MlClient::PatientData& patientData);

private:
    Ui::LoaderPage* ui;
    MainInterface* mainInterface_{};
    DataModel* inputData_{};
    DataModel* outputData_{};
    QElapsedTimer executionTimer_;

    Q_DISABLE_COPY_MOVE(LoaderPage)
};
