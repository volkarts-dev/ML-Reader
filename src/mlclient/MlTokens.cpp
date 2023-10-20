// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "MlTokens.h"

#include "Tools.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace {

QJsonArray makePidList(const QStringList& pids)
{
    QJsonArray json;
    for (const auto& pid : pids)
    {
        QJsonObject id;
        id["idType"_l1] = "pid"_l1;
        id["idString"_l1] = pid;
        json << id;
    }
    return json;
}

} // namespace

QJsonObject makeReadPatientToken(const QVersionNumber& apiVersion, QStringList pids, QStringList fields)
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
