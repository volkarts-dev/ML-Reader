// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "UserSettings.h"

const QString CfgLastAccessedDirectory = QStringLiteral("Main/LastAccessedDirectory");
const QString CfgCsvColumnSeparator = QStringLiteral("Main/Csv/ColumnSeparator");
const QString CfgCsvQuotingCharacter = QStringLiteral("Main/Csv/QuotingCharacter");
const QString CfgCsvCodec = QStringLiteral("Main/Csv/Codec");

const QString CfgCsvDefaultColumnSeparator = QStringLiteral(";");
const QString CfgCsvDefaultQuotingCharacter = QStringLiteral("\"");
const QString CfgCsvDefaultCodec = QStringLiteral("ISO-8859-1");


UserSettings::UserSettings()
{
}

QString UserSettings::stringValue(const QString& key) const
{
    return value(key).toString();
}

QByteArray UserSettings::byteArrayValue(const QString& key) const
{
    return value(key).toByteArray();
}

int UserSettings::intValue(const QString& key) const
{
    return value(key).toInt();
}

bool UserSettings::boolValue(const QString& key) const
{
    return value(key).toBool();
}
