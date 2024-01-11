// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MlClient.h"

#include "MlTokens.h"
#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Tools.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <utility>

class MlConversation : public QObject
{
    Q_OBJECT

public:
    MlConversation(MlClient* mlClient, QObject* parent = {}) :
        QObject{parent},
        mlClient_{mlClient}
    {
    }

    void start()
    {
        createSession();
    }

signals:
    void finishedError(const QString& error);

private:
    void createSession()
    {
        QString path = "/sessions"_l1;

        auto response = startRequest(HttpRequest::Method::POST, path, {});

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if (error || statusCode != 201)
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to create session. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";

                emit finishedError(statusCode != 404 ?
                            messageFromServer : tr("Mainzelliste not found on server. Check the BaseURL."));
                return;
            }

            auto responseObject = response->body().toJsonObject();

            sessionId_ = responseObject["sessionId"_l1].toString();

            createToken();

            response->deleteLater();
        });
    }

    void createToken()
    {
        QString path = "/sessions/"_l1 + sessionId_ + "/tokens"_l1;
        auto body = HttpBody::fromJson(createTokenObject());

        auto response = startRequest(HttpRequest::Method::POST, path, {}, body);

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if (error || statusCode != 201)
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to create token. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";

                emit finishedError(messageFromServer);
                return;
            }

            auto responseObject = response->body().toJsonObject();

            tokenId_ = responseObject["id"_l1].toString();

            doActualRequest();

            response->deleteLater();
        });
    }

protected:
    void deleteSession()
    {
        QString path = "/sessions/"_l1 + sessionId_;

        auto response = startRequest(HttpRequest::Method::DELETE, path, {});

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if (error || statusCode != 204)
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to delete session. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";
            }

            response->deleteLater();
        });
    }

    inline QString errorMessage(const HttpResponse* response)
    {
        const auto error = response->networkError();
        if (error < 200)
        {
            return tr("Could not connect to Mainzelliste server. Check internet connection or the BaseURL");
        }

        const auto& buffer = response->body().binaryData();
        if (buffer.trimmed().isEmpty())
            return response->networkErrorString();
        return QString::fromUtf8(buffer);
    }

    inline HttpResponse* startRequest(HttpRequest::Method method, const QString& path,
                                      const QUrlQuery& query, const HttpBody& body = {}) const
    {
        return mlClient_->http_->startRequest(mlClient_->createRequest(method, path, query, body));
    }

    const QString& sessionId() const { return sessionId_; }
    const QString& tokenId() const { return tokenId_; }

    virtual QJsonObject createTokenObject() = 0;
    virtual void doActualRequest() = 0;

private:
    MlClient* mlClient_;
    QString sessionId_{};
    QString tokenId_{};
};

// ********************************************************

class LoadPatientDataConversation : public MlConversation
{
    Q_OBJECT

public:
    LoadPatientDataConversation(QVersionNumber apiVersion, QStringList pids, QStringList fields,
                                MlClient* mlClient, QObject* parent = {}) :
        MlConversation{mlClient, parent},
        apiVersion_{std::move(apiVersion)},
        pids_{std::move(pids)},
        fields_{std::move(fields)}
    {
    }

    QJsonObject createTokenObject() override
    {
        return makeReadPatientToken(apiVersion_, pids_, fields_);
    }

    void doActualRequest() override
    {
        QString path = "/patients"_l1;
        QUrlQuery query{{QStringLiteral("tokenId"), tokenId()}};

        auto response = startRequest(HttpRequest::Method::GET, path, query);

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if (error || statusCode != 200)
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to get patient data. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";

                emit finishedError(messageFromServer);
                return;
            }

            auto responseObject = response->body().toJsonArray();

            auto patientData = parseResponse(responseObject);

            emit patientDataLoaded(patientData);

            deleteSession();

            response->deleteLater();
        });
    }

signals:
    void patientDataLoaded(const MlClient::PatientData& data);

private:
    MlClient::PatientData parseResponse(const QJsonArray& json)
    {
        MlClient::PatientData patientData;

        for (const auto& val : json)
        {
            const auto fields = val.toObject()["fields"_l1].toObject();
            const auto pid = findPid(val.toObject()["ids"_l1].toArray());

            MlClient::PatientRecord rec;

            rec.insert(MlClient::ID_TYPE, pid);

            for (auto it = fields.begin(); it != fields.end(); ++it)
            {
                rec.insert(it.key(), it.value().toString());
            }

            patientData << rec;
        }

        return patientData;
    }

    QString findPid(const QJsonArray& ids)
    {
        for (const auto& id : ids)
        {
            const auto idObj = id.toObject();
            if (idObj["idType"_l1].toString() == MlClient::ID_TYPE)
                return idObj["idString"_l1].toString();
        }
        return {};
    }

private:
    QVersionNumber apiVersion_;
    QStringList pids_;
    QStringList fields_;
};

// ********************************************************

class QueryPatientDataConversation : public MlConversation
{
    Q_OBJECT

public:
    QueryPatientDataConversation(QVersionNumber apiVersion, QHash<QString, QString> patientData, bool sureness,
                                 MlClient* mlClient, QObject* parent = {}) :
        MlConversation{mlClient, parent},
        apiVersion_{std::move(apiVersion)},
        patientData_{std::move(patientData)}
    {
        if (sureness)
            patientData_.insert("sureness"_l1, "true"_l1);
    }

    QJsonObject createTokenObject() override
    {
        return makeCreatePatientToken(apiVersion_);
    }

    void doActualRequest() override
    {
        QString path = "/patients"_l1;
        QUrlQuery query{{QStringLiteral("tokenId"), tokenId()}};
        auto body = HttpBody::urlEncodedFromHash(patientData_);

        auto response = startRequest(HttpRequest::Method::POST, path, query, body);

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if ((error && error != QNetworkReply::ContentConflictError) || (statusCode != 201 && statusCode != 409))
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to query patient data. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";

                emit finishedError(messageFromServer);
                return;
            }

            MlClient::QueryResult queryResult;

            const auto returnValue = response->body().toJson();
            if (returnValue.isArray())
            {
                parseResponse(queryResult, returnValue.array());
            }
            else
            {
                parseConflictResponse(queryResult, returnValue.object());
            }

            emit patientDataQueried(queryResult);

            deleteSession();

            response->deleteLater();
        });
    }

signals:
    void patientDataQueried(const MlClient::QueryResult& data);

private:
    void parseResponse(MlClient::QueryResult& queryResult, const QJsonArray& json)
    {
        for (const auto& idVal : json)
        {
            const auto idObject = idVal.toObject();

            const auto idType = idObject["idType"_l1].toString();
            if (idType == MlClient::ID_TYPE)
            {
                queryResult.pid = idObject["idString"_l1].toString();
                queryResult.tentative = idObject["tentative"_l1].toBool();
                break;
            }
        }
    }

    void parseConflictResponse(MlClient::QueryResult& queryResult, const QJsonObject& json)
    {
        const auto possibleMatches = json["possibleMatches"_l1].toArray();
        for (const auto& match : possibleMatches)
        {
            const auto matchObject = match.toObject();

            const auto idType = matchObject["idType"_l1].toString();
            if (idType != MlClient::ID_TYPE)
            {
                qCWarning(MLC_LOG_CAT) << "Received unknown id type" << idType;
                continue;
            }

            queryResult.possibleMatchPids << matchObject["idString"_l1].toString();
        }
    }

private:
    QVersionNumber apiVersion_;
    QHash<QString, QString> patientData_;
};

// ********************************************************

class EditPatientDataConversation : public MlConversation
{
    Q_OBJECT

public:
    EditPatientDataConversation(QVersionNumber apiVersion, QString pid, QHash<QString, QString> patientData,
                                 MlClient* mlClient, QObject* parent = {}) :
        MlConversation{mlClient, parent},
        apiVersion_{std::move(apiVersion)},
        pid_{std::move(pid)},
        patientData_{std::move(patientData)}
    {
    }

    QJsonObject createTokenObject() override
    {
        return makeEditPatientToken(apiVersion_, pid_);
    }

    void doActualRequest() override
    {
        QString path = "/patients/tokenId/"_l1 + tokenId();
        auto body = HttpBody::jsonObjectFromHash(patientData_);

        auto response = startRequest(HttpRequest::Method::PUT, path, {}, body);

        connect(response, &HttpResponse::finished, this,
                [this, response](QNetworkReply::NetworkError error, int statusCode)
        {
            if (error || statusCode != 204)
            {
                const auto messageFromServer = errorMessage(response);

                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to edit patient data. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                messageFromServer << "\n<<<";

                emit finishedError(messageFromServer);
                return;
            }

            emit patientDataEdited();

            deleteSession();

            response->deleteLater();
        });
    }

signals:
    void patientDataEdited();

private:
    QVersionNumber apiVersion_;
    QString pid_;
    QHash<QString, QString> patientData_;
};

// ********************************************************

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include "MlClient.moc"
#pragma GCC diagnostic pop

// ********************************************************

const QString MlClient::ID_TYPE = QStringLiteral("pid");

MlClient::MlClient(QString baseUrl, QVersionNumber apiVersion, QString apiKey, QObject* parent) :
    QObject{parent},
    baseUrl_{std::move(baseUrl)},
    apiVersion_{std::move(apiVersion)},
    apiKey_{std::move(apiKey)},
    http_{new HttpClient{this, this}}
{
}

void MlClient::loadPatientData(const QStringList& pids, const QStringList& fields)
{
    auto conversation = new LoadPatientDataConversation(apiVersion_, pids, fields, this, this);
    connect(conversation, &LoadPatientDataConversation::finishedError, this, [this] (const QString& error) {
        emit patientDataLoadingFailed(error);
    });
    connect(conversation, &LoadPatientDataConversation::patientDataLoaded, this, [this] (const PatientData& data) {
        emit patientDataLoaded(data);
    });
    conversation->start();
}

void MlClient::queryPatientData(const QHash<QString, QString>& patientData, bool sureness)
{
    auto conversation = new QueryPatientDataConversation(apiVersion_, patientData, sureness, this, this);
    connect(conversation, &QueryPatientDataConversation::finishedError, this, [this] (const QString& error) {
        emit patientDataQueringFailed(error);
    });
    connect(conversation, &QueryPatientDataConversation::patientDataQueried, this, [this] (const QueryResult& data) {
        emit patientDataQueried(data);
    });
    conversation->start();
}

void MlClient::editPatientData(const QString& pid, const QHash<QString, QString>& patientData)
{
    auto conversation = new EditPatientDataConversation(apiVersion_, pid, patientData, this, this);
    connect(conversation, &EditPatientDataConversation::finishedError, this, [this] (const QString& error) {
        emit patientDataEditingFailed(error);
    });
    connect(conversation, &EditPatientDataConversation::patientDataEdited, this, [this] () {
        emit patientDataEdited();
    });
    conversation->start();
}

bool MlClient::askRecoverableError(const QString& title, const QString& message)
{
    Q_UNUSED(title)
    Q_UNUSED(message)
    return true;
}

HttpRequest MlClient::createRequest(HttpRequest::Method method, const QString& path,
                                    const QUrlQuery& query, const HttpBody& body)
{
    QUrl url{baseUrl_};
    url.setPath(url.path() + path);

    QUrlQuery urlQuery{url};
    auto queryItems = urlQuery.queryItems();
    queryItems.append(query.queryItems());
    urlQuery.setQueryItems(queryItems);
    url.setQuery(urlQuery);

    HttpRequest req(method, url);
    req.addHeader("mainzellisteApiKey", apiKey_);
    req.addHeader("mainzellisteApiVersion", apiVersion_.toString());

    req.setBody(body);

    return req;
}
