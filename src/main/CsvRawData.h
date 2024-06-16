// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <qtcsv/abstractdata.h>
#include <QList>

class CsvRawData : public QtCSV::AbstractData
{
public:
    CsvRawData();
    CsvRawData(QList<QStringList> data, bool skipFirstRow);

    QList<QStringList> data() const { return rawData_; }

    void addEmptyRow() override;
    void addRow(const QStringList& values) override;
    void clear() override;
    bool isEmpty() const override;
    qsizetype rowCount() const override;
    QList<QString> rowValues(qsizetype row) const override;

private:
    QList<QStringList> rawData_;
    bool skipFirstRow_{false};
};
