// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include "MlClient.h"
#include <QWidget>

class DataModel;
class MainInterface;

namespace Ui {
class EditorPage;
}

class EditorPage : public QWidget
{
    Q_OBJECT

public:
    explicit EditorPage(QWidget* parent = {});
    ~EditorPage() override;

    void setMainInterface(MainInterface* mainInterface) { mainInterface_ = mainInterface; }

    void startEditing(const QString& pid);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectedEnpointChanged(int index);
    void onLoadIDataBtnClicked();
    void onSaveBtnClicked();
    void onAbortBtnClicked();
    void onPatientDataLoadingFailed(const QString& error);
    void onPatientDataLoaded(const MlClient::PatientData& patientData);
    void onPatientDataEditingFailed(const QString& error);
    void onPatientDataEdited();

private:
    void setup();
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();
    void startEditing();

private:
    Ui::EditorPage* ui;
    MainInterface* mainInterface_{};
    QString loadedPatientPid_{};

    Q_DISABLE_COPY_MOVE(EditorPage)
};
