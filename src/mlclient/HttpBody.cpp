// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "HttpBody.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

HttpBody HttpBody::fromJson(const QJsonArray& json)
{
    QJsonDocument doc{json};
    return {QStringLiteral("application/json"), doc.toJson()};
}

HttpBody HttpBody::fromJson(const QJsonObject& json)
{
    QJsonDocument doc{json};
    return {QStringLiteral("application/json"), doc.toJson()};
}

HttpBody::HttpBody()
{
}

HttpBody::HttpBody(QString contentType, QByteArray binaryData) :
    contentType_{std::move(contentType)},
    binaryData_{std::move(binaryData)}
{
}

QJsonArray HttpBody::toJsonArray() const
{
    auto json = QJsonDocument::fromJson(binaryData_);
    return json.array();
}

QJsonObject HttpBody::toJsonObject() const
{
    auto json = QJsonDocument::fromJson(binaryData_);
    return json.object();
}
