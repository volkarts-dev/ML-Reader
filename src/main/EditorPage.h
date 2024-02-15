// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MlClient.h"
#include <QWidget>

class DataModel;
class MainWindow;

namespace Ui {
class EditorPage;
}

class EditorPage : public QWidget
{
    Q_OBJECT

public:
    explicit EditorPage(QWidget* parent = {});
    ~EditorPage() override;

    void initialize(MainWindow* mainWindow);

    void startEditing(const QString& pid);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onLoadIDataBtnClicked();
    void onSaveBtnClicked();
    void onAbortBtnClicked();
    void onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData);
    void onPatientDataEditingDone(const MlClient::Error& error);
    void onEndpointConfigChanged();
    void onSelectedEndpointChanged(int index);

private:
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void startEditing();
    void reloadDynamicForm(int endpointIndex);

private:
    Ui::EditorPage* ui;
    MainWindow* mainWindow_{};
    QString loadedPatientPid_{};

    Q_DISABLE_COPY_MOVE(EditorPage)
};
