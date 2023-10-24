// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "HttpBody.h"
#include <QNetworkReply>
#include <QObject>

class QAuthenticator;
class QSslError;

class HttpResponse : public QObject
{
    Q_OBJECT

private:
    HttpResponse(QNetworkReply* reply, QObject* parent = {});

public:
    QString networkErrorString() const { return reply_->errorString(); }
    const HttpBody& body() const { return body_; }

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void finished(QNetworkReply::NetworkError error, int statusCode);

private slots:
    void onReplyReadyRead();
    void onReplyFinished();

private:
    QNetworkReply* reply_{};
    QByteArray receiveBuffer_; // TODO use temp file
    HttpBody body_;

    friend class HttpClient;
};
