// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QWidget>

class DataModel;
class MainInterface;

namespace Ui {
class QueryPage;
}

class QueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit QueryPage(QWidget* parent = {});
    ~QueryPage() override;

    void setMainInterface(MainInterface* mainInterface) { mainInterface_ = mainInterface; }

public slots:
    void handleEndpointConfigChanged();
    void setSelectedEndpoint(int index);

signals:
    void selectedEndpointChanged(int index);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectedEndpointChanged(int index);
    void onExecuteButtonClicked();
    void onEditPatientBtnClicked();
    void onCopyPidBtnClicked();
    void onCreateAnywayBtnClicked();
    void onCustomMenuRequested(const QPoint& position);
    void onPatientDataQueringDone(const MlClient::Error& error, const MlClient::QueryResult& result);
    void onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData);
    void onPossibleMatchesDoubleClicked(const QModelIndex& index);

private:
    void setup();
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void execute(bool sureness);
    void reloadDynamicForm(int endpointIndex);

private:
    Ui::QueryPage* ui;
    MainInterface* mainInterface_{};
    DataModel* possibleMatchesModel_;

    Q_DISABLE_COPY_MOVE(QueryPage)
};
