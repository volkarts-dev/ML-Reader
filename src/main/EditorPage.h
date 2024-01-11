// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

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

public slots:
    void setSelectedEndpoint(int index);

signals:
    void selectedEndpointChanged(int index);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectedEndpointChanged(int index);
    void onLoadIDataBtnClicked();
    void onSaveBtnClicked();
    void onAbortBtnClicked();
    void onPatientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& patientData);
    void onPatientDataEditingDone(const MlClient::Error& error);

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
