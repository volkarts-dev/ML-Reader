// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MlTokens.h"

#include "Tools.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace {

QJsonObject makePidObject(const QString& pid)
{
    QJsonObject id;
    id["idType"_l1] = "pid"_l1;
    id["idString"_l1] = pid;
    return id;
}

QJsonArray makePidList(const QStringList& pids)
{
    QJsonArray json;
    for (const auto& pid : pids)
    {
        json << makePidObject(pid);
    }
    return json;
}

} // namespace

QJsonObject makeReadPatientToken(const QVersionNumber& apiVersion, const QStringList& pids, const QStringList& fields)
{
    QJsonObject data;
    data["searchIds"_l1] = makePidList(pids);
    data["resultIds"_l1] = QJsonArray({"pid"_l1});
    data["resultFields"_l1] = QJsonArray::fromStringList(fields);

    QJsonObject token;
    token["type"_l1] = "readPatients"_l1;
    token["data"_l1] = data;

    return token;
}

QJsonObject makeCreatePatientToken(const QVersionNumber& apiVersion)
{
    QJsonObject data;
    data["idTypes"_l1] = QJsonArray({"pid"_l1});

    QJsonObject token;
    token["type"_l1] = "addPatient"_l1;
    token["data"_l1] = data;

    return token;
}

QJsonObject makeEditPatientToken(const QVersionNumber& apiVersion, const QString& pid)
{
    QJsonObject data;
    data["patientId"_l1] = makePidObject(pid);
    // data["redirect"_l1] =
    // data["fields"_l1] =

    QJsonObject token;
    token["type"_l1] = "editPatient"_l1;
    token["data"_l1] = data;

    return token;
}
