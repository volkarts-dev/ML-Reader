// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "HttpBody.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>

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

HttpBody HttpBody::jsonObjectFromHash(const QHash<QString, QString>& data)
{
    QJsonObject object;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        object[it.key()] = it.value();
    }
    return fromJson(object);
}

HttpBody HttpBody::urlEncodedFromHash(const QHash<QString, QString>& data)
{
    QByteArray buffer;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        buffer += QUrl::toPercentEncoding(it.key());
        buffer += '=';
        buffer += QUrl::toPercentEncoding(it.value());
        buffer += '&';
    }

    return {QStringLiteral("application/x-www-form-urlencoded"), buffer};
}

HttpBody::HttpBody()
{
}

HttpBody::HttpBody(QString contentType, QByteArray binaryData) :
    contentType_{std::move(contentType)},
    binaryData_{std::move(binaryData)}
{
}

QJsonDocument HttpBody::toJson() const
{
    return QJsonDocument::fromJson(binaryData_);
}

QJsonArray HttpBody::toJsonArray() const
{
    return toJson().array();
}

QJsonObject HttpBody::toJsonObject() const
{
    return toJson().object();
}
