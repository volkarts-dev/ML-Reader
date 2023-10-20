// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QVariant>
#include <QMetaType>

class QSettings;

class EndpointConfig
{
public:
    enum class Field
    {
        Uuid,
        Name,
        BaseURL,
        ApiVersion,
        Fields,
        SaveApiKey,
        _Count,
    };

    EndpointConfig(const QString& name = {});

    void load(const QSettings& s);
    void save(QSettings& s);

    QVariant value(Field field) const;
    void setValue(Field field, const QVariant& value);

private:
    QVector<QVariant> data_;
};

Q_DECLARE_METATYPE(EndpointConfig)
