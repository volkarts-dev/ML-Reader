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
    using PatientRecord = QHash<QString, QString>;
    using PatientData = QList<PatientRecord>;

public:
    MlClient(QString baseUrl, QVersionNumber apiVersion, QString apiKey, QObject* parent = {});

    void loadPatientData(const QStringList& pids, const QStringList& fields);

    bool askRecoverableError(const QString& title, const QString& message) override;

signals:
    void patientDataLoaded(bool result, const MlClient::PatientData& data);

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
