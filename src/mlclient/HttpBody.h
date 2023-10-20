// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

class QJsonArray;
class QJsonObject;

class HttpBody
{
public:
    static HttpBody fromJson(const QJsonArray& json);
    static HttpBody fromJson(const QJsonObject& json);

public:
    HttpBody();
    HttpBody(QString contentType, QByteArray binaryData);

    QString contentType() const { return contentType_; }
    void setContentType(const QString& contentType) { contentType_ = contentType; }

    QByteArray binaryData() const { return binaryData_; }
    void setBinaryData(const QByteArray& binaryData) { binaryData_ = binaryData; }

    bool isNull() const { return contentType_.isEmpty(); }

    QJsonArray toJsonArray() const;
    QJsonObject toJsonObject() const;

private:
    QString contentType_;
    QByteArray binaryData_;
};
