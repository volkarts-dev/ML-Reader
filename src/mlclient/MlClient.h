// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "HttpBody.h"
#include "HttpRequest.h"
#include "HttpUserDelegate.h"
#include <QObject>
#include <QVersionNumber>

class HttpClient;
class MlConversation;

class MlClient : public QObject, public HttpUserDelegate
{
    Q_OBJECT

public:
    struct QueryResult
    {
        QString pid{};
        bool tentative{};
        QStringList possibleMatchPids{};
    };

    using PatientRecord = QHash<QString, QString>;
    using PatientData = QList<PatientRecord>;

    static const QString ID_TYPE;

public:
    MlClient(QString baseUrl, QVersionNumber apiVersion, QString apiKey, QObject* parent = {});

    void loadPatientData(const QStringList& pids, const QStringList& fields);
    void queryPatientData(const QHash<QString, QString>& patientData);
    void editPatientData(const QString& pid, const QHash<QString, QString>& patientData);

    bool askRecoverableError(const QString& title, const QString& message) override;

signals:
    void patientDataLoadingFailed(const QString& error);
    void patientDataLoaded(const MlClient::PatientData& data);

    void patientDataQueringFailed(const QString& error);
    void patientDataQueried(const MlClient::QueryResult& result);

    void patientDataEditingFailed(const QString& error);
    void patientDataEdited();

private:
    HttpRequest createRequest(HttpRequest::Method method, const QString& path,
                              const QUrlQuery& query, const HttpBody& body = {});

private slots:

private:
    QString baseUrl_;
    QVersionNumber apiVersion_;
    QString apiKey_;
    HttpClient* http_;

    friend MlConversation;
};

Q_DECLARE_METATYPE(MlClient::QueryResult)
