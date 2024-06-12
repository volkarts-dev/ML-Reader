// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EndpointConfig.h"

#include "Tools.h"
#include <QSettings>
#include <QUuid>

namespace {

const auto CfgUuid = QString("Uuid");
const auto CfgName = QString("Name");
const auto CfgBaseURL = QString("BaseURL");
const auto CfgApiVersion = QString("ApiVersion");
const auto CfgFields = QString("Fields");
const auto CfgSaveApiKey = QString("SaveApiKey");

QString stripTrailingSlash(QString url)
{
    while (url.endsWith(QLatin1Char('/')))
    {
        url.remove(url.length() - 1, 1);
    }
    return url;
}

} // namespace

EndpointConfig::EndpointConfig(const QString& name)
{
    data_.resize(static_cast<int>(Field::_Count));
    setValue(Field::Name, name);
}

void EndpointConfig::load(const QSettings& s)
{
    auto id = s.value(CfgUuid).toUuid();
    if (id.isNull())
        id = QUuid::createUuid();

    data_[toInt(Field::Uuid)] = id;
    data_[toInt(Field::Name)] = s.value(CfgName).toString();
    data_[toInt(Field::BaseURL)] = stripTrailingSlash(s.value(CfgBaseURL).toString());
    data_[toInt(Field::ApiVersion)] = s.value(CfgApiVersion).toString();
    data_[toInt(Field::Fields)] = s.value(CfgFields).value<QStringList>();

    if (data_[toInt(Field::ApiVersion)].toString().isEmpty())
        data_[toInt(Field::ApiVersion)] = QStringLiteral("2.2");

    auto saveApiKey = s.value(CfgSaveApiKey);
    if (!saveApiKey.isValid())
        saveApiKey.setValue(true);
    data_[toInt(Field::SaveApiKey)] = saveApiKey.toBool();
}

void EndpointConfig::save(QSettings& s)
{
    if (data_[toInt(Field::Uuid)].toUuid().isNull())
        data_[toInt(Field::Uuid)] = QUuid::createUuid();

    s.setValue(CfgUuid, data_[toInt(Field::Uuid)]);
    s.setValue(CfgName, data_[toInt(Field::Name)]);
    s.setValue(CfgBaseURL, data_[toInt(Field::BaseURL)]);
    s.setValue(CfgApiVersion, data_[toInt(Field::ApiVersion)]);
    s.setValue(CfgFields, data_[toInt(Field::Fields)]);
    s.setValue(CfgSaveApiKey, data_[toInt(Field::SaveApiKey)]);
}

QVariant EndpointConfig::value(EndpointConfig::Field field) const
{
    return data_[static_cast<int>(field)];
}

void EndpointConfig::setValue(EndpointConfig::Field field, const QVariant& value)
{
    if (field == Field::BaseURL)
    {
        data_[static_cast<int>(field)] = stripTrailingSlash(value.toString());
    }
    else
    {
        data_[static_cast<int>(field)] = value;
    }
}
