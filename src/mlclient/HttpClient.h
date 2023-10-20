// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "HttpBody.h"
#include <QObject>
#include <QNetworkAccessManager>

class HttpUserDelegate;
class HttpRequest;
class HttpResponse;

class HttpClient : public QObject
{
    Q_OBJECT

public:

public:
    HttpClient(HttpUserDelegate* delegate, QObject* parent = {});

    HttpResponse* startRequest(const HttpRequest& request);

private slots:
    void onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    HttpUserDelegate* delegate_;
    QNetworkAccessManager qnam_;
};
