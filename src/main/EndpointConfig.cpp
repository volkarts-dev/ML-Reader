// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EndpointConfig.h"

#include "Tools.h"
#include <QSettings>
#include <QUuid>

namespace {
QString stripTrailingSlash(QString url)
{
    while (url.endsWith(QLatin1Char('/')))
    {
        url.remove(url.length() - 1, 1);
    }
    return url;
}

}
EndpointConfig::EndpointConfig(const QString& name)
{
    data_.resize(static_cast<int>(Field::_Count));
    setValue(Field::Name, name);
}

void EndpointConfig::load(const QSettings& s)
{
    auto id = s.value(QStringLiteral("Uuid")).toUuid();
    if (id.isNull())
        id = QUuid::createUuid();

    data_[toInt(Field::Uuid)] = id;
    data_[toInt(Field::Name)] = s.value(QStringLiteral("Name")).toString();
    data_[toInt(Field::BaseURL)] = stripTrailingSlash(s.value(QStringLiteral("BaseURL")).toString());
    data_[toInt(Field::ApiVersion)] = s.value(QStringLiteral("ApiVersion")).toString();
    data_[toInt(Field::Fields)] = s.value(QStringLiteral("Fields")).value<QStringList>();

    if (data_[toInt(Field::ApiVersion)].toString().isEmpty())
        data_[toInt(Field::ApiVersion)] = QStringLiteral("2.2");

    auto saveApiKey = s.value(QStringLiteral("SaveApiKey"));
    if (!saveApiKey.isValid())
        saveApiKey.setValue(true);
    data_[toInt(Field::SaveApiKey)] = saveApiKey.toBool();
}

void EndpointConfig::save(QSettings& s)
{
    if (data_[toInt(Field::Uuid)].toUuid().isNull())
        data_[toInt(Field::Uuid)] = QUuid::createUuid();

    s.setValue(QStringLiteral("Uuid"), data_[toInt(Field::Uuid)]);
    s.setValue(QStringLiteral("Name"), data_[toInt(Field::Name)]);
    s.setValue(QStringLiteral("BaseURL"), data_[toInt(Field::BaseURL)]);
    s.setValue(QStringLiteral("ApiVersion"), data_[toInt(Field::ApiVersion)]);
    s.setValue(QStringLiteral("Fields"), data_[toInt(Field::Fields)]);
    s.setValue(QStringLiteral("SaveApiKey"), data_[toInt(Field::SaveApiKey)]);
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
