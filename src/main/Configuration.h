// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QVariant>

class QDataWidgetMapper;
class QObject;
class QSettings;

class Configuration
{
public:
    enum class Key
    {
        LastAccessedDirectory,
        CsvColumnSeparator,
        CsvQuotingCharacter,
        CsvCodec,

        _Count,
    };

    Configuration();

    void setValue(Key key, const QVariant& value);
    QVariant value(Key key) const;
    QString stringValue(Key key) const;
    int intValue(Key key) const;
    bool boolValue(Key key) const;

    //QDataWidgetMapper* createWidgetMapper(QObject* parent);
};
