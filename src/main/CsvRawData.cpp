// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "CsvRawData.h"

CsvRawData::CsvRawData()
{
}

CsvRawData::CsvRawData(QList<QStringList> data, bool skipFirstRow) :
    rawData_{std::move(data)},
    skipFirstRow_{skipFirstRow}
{
}

void CsvRawData::addEmptyRow()
{
    rawData_.append(QStringList{});
}

void CsvRawData::addRow(const QStringList& values)
{
    rawData_.append(values);
}

void CsvRawData::clear()
{
    rawData_.clear();
}

bool CsvRawData::isEmpty() const
{
    return skipFirstRow_ ? rawData_.count() <= 1 : rawData_.isEmpty();
}

int CsvRawData::rowCount() const
{
    return qMax(0, rawData_.count() - (skipFirstRow_ ? 1 : 0));
}

QStringList CsvRawData::rowValues(const int& row) const
{
    return rawData_[row + (skipFirstRow_ ? 1 : 0)];
}
