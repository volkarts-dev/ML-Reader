// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include "HttpBody.h"
#include <QHash>
#include <QString>
#include <QUrl>

class HttpRequest
{
public:
    enum class Method
    {
        GET,
        POST,
        PUT,
        DELETE,
    };

public:
    HttpRequest(Method method, const QUrl& url);

    Method method() const { return method_; }
    QUrl url() const { return url_; }

    QHash<QString, QString> headers() const { return headers_; }
    void addHeader(QString name, QString value);

    HttpBody body() const { return body_; }
    void setBody(const HttpBody& body);

private:
    Method method_;
    QUrl url_;
    QHash<QString, QString> headers_;
    HttpBody body_;
};

