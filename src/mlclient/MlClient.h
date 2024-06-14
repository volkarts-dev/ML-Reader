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
    struct Error
    {
        QString message{};
        Error() = default;
        Error(QString m) : message{std::move(m)} {}
        operator bool() const { return !message.isEmpty(); }
    };

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
    void queryPatientData(const QHash<QString, QString>& patientData, bool sureness);
    void editPatientData(const QString& pid, const QHash<QString, QString>& patientData);

    bool askRecoverableError(const QString& title, const QString& message) override;

signals:
    void logMessage(QtMsgType type, const QString& message);

    void patientDataLoadingDone(const MlClient::Error& error, const MlClient::PatientData& data);
    void patientDataQueringDone(const MlClient::Error& error, const MlClient::QueryResult& result);
    void patientDataEditingDone(const MlClient::Error& error);

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

Q_DECLARE_METATYPE(MlClient::Error)
Q_DECLARE_METATYPE(MlClient::QueryResult)
Q_DECLARE_METATYPE(MlClient::PatientData)
