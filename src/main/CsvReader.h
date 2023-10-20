// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

class Configuration;
class DataModel;
class QIODevice;

class CsvReader : public QObject
{
    Q_OBJECT

public:
    explicit CsvReader(QObject* parent = {});
    ~CsvReader() override;

    bool read(QIODevice& input, DataModel* model);
    bool write(QIODevice& output, bool withHeader, DataModel* model);
};
