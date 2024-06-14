// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MlClientTools.h"

#include "Application.h"
#include "EndpointConfigModel.h"
#include "MlClient.h"
#include "Tools.h"
#include <QVersionNumber>

MlClient* createMlClientIntern(int endpointIndex, const QString& apiKey)
{
    const auto model = app()->endpointConfigModel();

    const auto baseUrl = model->data(
                model->index(endpointIndex, toInt(EndpointConfig::Field::BaseURL)),
                Qt::DisplayRole).toString();
    const auto apiVersion = model->data(
                model->index(endpointIndex, toInt(EndpointConfig::Field::ApiVersion)),
                Qt::DisplayRole).toString();

    return new MlClient{baseUrl, QVersionNumber::fromString(apiVersion), apiKey};
}

void deleteSenderMlClient(QObject* sender)
{
    auto mlClient = qobject_cast<MlClient*>(sender);
    Q_ASSERT(mlClient);
    mlClient->deleteLater();
}
