// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QSettings>

class UserSettings : public QSettings
{
public:
    UserSettings();

    QString stringValue(const QString& key) const;
    QByteArray byteArrayValue(const QString& key) const;
    int intValue(const QString& key) const;
    bool boolValue(const QString& key) const;
};

extern const QString CfgLastAccessedDirectory;
extern const QString CfgCsvColumnSeparator;
extern const QString CfgCsvQuotingCharacter;
extern const QString CfgCsvCodec;

extern const QString CfgCsvDefaultColumnSeparator;
extern const QString CfgCsvDefaultQuotingCharacter;
extern const QString CfgCsvDefaultCodec;
