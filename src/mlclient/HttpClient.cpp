// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "HttpClient.h"

#include "HttpUserDelegate.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <QAuthenticator>
#include <QNetworkReply>

namespace {

void setHeaders(QNetworkRequest& request, const QHash<QString, QString>& headers)
{
    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
}

} // namespace

HttpClient::HttpClient(HttpUserDelegate* delegate, QObject* parent) :
    QObject{parent},
    delegate_{delegate}
{
    connect(&qnam_, &QNetworkAccessManager::authenticationRequired, this, &HttpClient::onAuthenticationRequired);
    connect(&qnam_, &QNetworkAccessManager::sslErrors, this, &HttpClient::onSslErrors);
}

HttpResponse* HttpClient::startRequest(const HttpRequest& request)
{
    switch (request.method())
    {
        using enum HttpRequest::Method;

        case GET:
        {
            QNetworkRequest req{request.url()};
            setHeaders(req, request.headers());
            return new HttpResponse{qnam_.get(req), this};
            break;
        }

        case POST:
        {
            QNetworkRequest req{request.url()};
            setHeaders(req, request.headers());
            if (!request.body().isNull())
                req.setHeader(QNetworkRequest::ContentTypeHeader, request.body().contentType());
            return new HttpResponse{qnam_.post(req, request.body().binaryData()), this};
            break;
        }

        case PUT:
        {
            QNetworkRequest req{request.url()};
            setHeaders(req, request.headers());
            if (!request.body().isNull())
                req.setHeader(QNetworkRequest::ContentTypeHeader, request.body().contentType());
            return new HttpResponse{qnam_.put(req, request.body().binaryData()), this};
            break;
        }

        case DELETE:
        {
            QNetworkRequest req{request.url()};
            setHeaders(req, request.headers());
            return new HttpResponse{qnam_.deleteResource(req), this};
            break;
        }
    }

    Q_UNREACHABLE();
}


void HttpClient::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    Q_UNUSED(authenticator)

    reply->abort();
}

void HttpClient::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
    QString errorString;
    for (const QSslError &error : errors) {
        if (!errorString.isEmpty())
            errorString += '\n';
        errorString += error.errorString();
    }

    if (delegate_->askRecoverableError(tr("SSL Errors"), tr("One or more SSL errors has occurred:\n%1").arg(errorString)))
    {
        reply->ignoreSslErrors();
    }
}
