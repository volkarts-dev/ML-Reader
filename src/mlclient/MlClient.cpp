// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "MlClient.h"

#include "MlTokens.h"
#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Tools.h"
#include <QJsonArray>
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
    void finishedError();

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
                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to create session. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                QString::fromUtf8(response->receiveBuffer()) << "\n<<<";
                emit finishedError();
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
                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to create token. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                QString::fromUtf8(response->receiveBuffer()) << "\n<<<";
                emit finishedError();
                return;
            }

            auto responseObject = response->body().toJsonObject();

            tokenId_ = responseObject["id"_l1].toString();

            doActualRequest();

            response->deleteLater();
        });
    }

protected:
    //inline MlClient* mlClient() const { return mlClient_; }
    //inline HttpClient* http() const { return mlClient_->http_; }
    //inline HttpRequest createRequest() const { return mlClient_->createRequest()}
    inline HttpResponse* startRequest(HttpRequest::Method method, const QString& path,
                                      const QUrlQuery& query, const HttpBody& body = {}) const
    {
        return mlClient_->http_->startRequest(mlClient_->createRequest(method, path, query, body));
    }

    const QString& sessionId() const { return sessionId_; }
    const QString& tokenId() const { return tokenId_; }
    //void setSessionId(const QString& newSessionId) { sessionId_ = newSessionId; }

    virtual QJsonObject createTokenObject() = 0;
    virtual void doActualRequest() = 0;

private:
    MlClient* mlClient_;
    QString sessionId_{};
    QString tokenId_{};
};

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
                qCWarning(MLC_LOG_CAT)
                        .nospace().noquote() << "Failed to get patient data. " <<
                                                "Error:" << error << ", Status:" << statusCode << "\n>>>\n" <<
                                                QString::fromUtf8(response->receiveBuffer()) << "\n<<<";
                emit finishedError();
                return;
            }

            auto responseObject = response->body().toJsonArray();

            auto patientData = parseResponse(responseObject);

            emit patientDataLoaded(patientData);

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

            rec.insert("pid"_l1, pid);

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
            if (idObj["idType"_l1].toString() == "pid"_l1)
                return idObj["idString"_l1].toString();
        }
        return {};
    }

private:
    QVersionNumber apiVersion_;
    QStringList pids_;
    QStringList fields_;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include "MlClient.moc"
#pragma GCC diagnostic pop

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
    connect(conversation, &LoadPatientDataConversation::finishedError, this, [this] () {
        emit patientDataLoaded(false, {});
    });
    connect(conversation, &LoadPatientDataConversation::patientDataLoaded, this, [this] (const PatientData& data) {
        emit patientDataLoaded(true, data);
    });
    conversation->start();
}

bool MlClient::askRecoverableError(const QString& title, const QString& message)
{
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
