// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include "MlClient.h"
#include <QWidget>

class DataModel;
class MessageView;

namespace Ui {
class QueryPage;
}

class QueryPage : public QWidget
{
    Q_OBJECT

public:
    explicit QueryPage(QWidget* parent = {});
    ~QueryPage() override;

    void setMessageView(MessageView* messageView) { messageView_ = messageView; }

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onSelectedEnpointChanged(int index);
    void onExecuteButtonClicked();
    void onEditPatientBtnClicked();
    void onCopyPidBtnClicked();
    void onPatientDataQueringFailed(const QString& error);
    void onPatientDataQueried(const MlClient::QueryResult& result);
    void onPatientDataLoadingFailed(const QString& error);
    void onPatientDataLoaded(const MlClient::PatientData& patientData);

private:
    void setup();
    void loadWidgetState();
    void saveWidgetState();
    void updateUiState();

private:
    Ui::QueryPage* ui;
    MessageView* messageView_{};
    DataModel* possibleMatchesModel_;

    Q_DISABLE_COPY_MOVE(QueryPage)
};
