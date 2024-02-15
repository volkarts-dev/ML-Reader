// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QWidget>

class DataModel;
class MainWindow;

namespace Ui {
class QueryPage;
}

class QueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit QueryPage(QWidget* parent = {});
    ~QueryPage() override;

    void initialize(MainWindow* mainWindow);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onExecuteButtonClicked();
    void onEditPatientBtnClicked();
    void onCopyPidBtnClicked();
    void onCreateAnywayBtnClicked();
    void onCustomMenuRequested(const QPoint& position);
    void onPatientDataQueringDone(const MlClient::Error& error, const MlClient::QueryResult& result);
    void onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData);
    void onPossibleMatchesDoubleClicked(const QModelIndex& index);
    void onEndpointConfigChanged();
    void onSelectedEndpointChanged(int index);

private:
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void execute(bool sureness);
    void reloadDynamicForm(int endpointIndex);

private:
    Ui::QueryPage* ui;
    MainWindow* mainWindow_{};
    DataModel* possibleMatchesModel_;

    Q_DISABLE_COPY_MOVE(QueryPage)
};
