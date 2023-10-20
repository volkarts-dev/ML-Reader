// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "DataModel.h"

namespace {

QString autoColName(int section)
{
    QString out;

    while (section >= 26)
    {
        out = QLatin1Char(static_cast<char>((section % 26) + 65)) + out;
        section /= 26;
    }
    out = QLatin1Char(static_cast<char>((section % 26) + 65)) + out;

    return out;
}

} // namespace

DataModel::DataModel(QObject* parent) :
    QAbstractTableModel{parent}
{
}

bool DataModel::detectFirstRowHeader(const QList<QStringList>& data)
{
    if (data.size() == 0)
        return false;

    const auto& firstRow = data[0];

    detectedPidColumn_ = -1;

    for (int i = 0; i < firstRow.size(); ++i)
    {
        if (firstRow[i].compare(QLatin1String("PID"), Qt::CaseInsensitive) == 0)
        {
            detectedPidColumn_ = i;
            return true;
        }
    }

    if (firstRow.count() == 1)
        detectedPidColumn_ = 0;

    return false;
}

void DataModel::setModelData(const QList<QStringList>& data, bool headerDetect)
{
    bool firstRowHeaderChanging = false;
    bool firstRowHeader = firstRowHeader_;

    if (headerDetect)
        firstRowHeader = detectFirstRowHeader(data); ;

    beginResetModel();

    data_ = data;

    if (firstRowHeader != firstRowHeader_)
    {
        firstRowHeader_ = firstRowHeader;
        firstRowHeaderChanging = true;
    }

    endResetModel();

    if (firstRowHeaderChanging)
        emit firstRowHeaderChanged(firstRowHeader_);
}

void DataModel::setFirstRowHeader(bool firstRowHeader)
{
    if (firstRowHeader == firstRowHeader_)
        return;

    beginResetModel();
    firstRowHeader_ = firstRowHeader;
    endResetModel();

    emit firstRowHeaderChanged(firstRowHeader_);
}

bool DataModel::hasData() const
{
    return rowCount() > 0;
}

int DataModel::rowCount(const QModelIndex& parent) const
{
    int cnt = data_.length();
    if (firstRowHeader_ && cnt > 0)
        --cnt;
    return cnt;
}

int DataModel::columnCount(const QModelIndex& parent) const
{
    return data_.length() == 0 ? 0 : data_[0].length();
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QAbstractTableModel::headerData(section, orientation, role);

    if (orientation == Qt::Horizontal)
    {
        if (firstRowHeader_ && !data_.isEmpty() && section < data_[0].length())
            return data_[0][section];
        else
            return autoColName(section);
    }
    else
    {
        return section + 1;
    }
}

QVariant DataModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return {};

    int row = index.row();
    if (firstRowHeader_)
        ++row;

    auto rd = data_[row];
    if (index.column() >= rd.length())
        return {};

    return rd[index.column()];
}
