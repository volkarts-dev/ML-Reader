// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EndpointConfigModel.h"

#include <QSettings>

EndpointConfigModel::EndpointConfigModel(QObject* parent) : QAbstractTableModel(parent)
{

}

EndpointConfigModel::~EndpointConfigModel()
{
    save();
}

int EndpointConfigModel::rowCount(const QModelIndex& parent) const
{
    return configs_.size();
}

int EndpointConfigModel::columnCount(const QModelIndex& parent) const
{
    return static_cast<int>(EndpointConfig::Field::_Count);
}

QVariant EndpointConfigModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    Q_ASSERT(index.row() >= 0 && index.row() < rowCount({}));
    Q_ASSERT(index.column() >= 0 && index.column() < columnCount({}));

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    const auto& c = configs_[index.row()];

    return c.value(static_cast<EndpointConfig::Field>(index.column()));
}

Qt::ItemFlags EndpointConfigModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool EndpointConfigModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return {};

    Q_ASSERT(index.row() >= 0 && index.row() < rowCount({}));
    Q_ASSERT(index.column() >= 0 && index.column() < columnCount({}));

    if (role != Qt::EditRole)
        return false;

    auto& c = configs_[index.row()];

    c.setValue(static_cast<EndpointConfig::Field>(index.column()), value);

    emit dataChanged(index, index, {role});

    return true;
}

int EndpointConfigModel::addConfig(const EndpointConfig& config)
{
    int newRow = configs_.size();

    beginInsertRows({}, newRow, newRow);

    configs_ << config;

    endInsertRows();

    return newRow;
}

void EndpointConfigModel::removeConfig(int index)
{
    Q_ASSERT(index >= 0 && index < configs_.size());

    beginRemoveRows({}, index, index);

    configs_.removeAt(index);

    endRemoveRows();
}

bool EndpointConfigModel::load()
{
    beginResetModel();

    configs_.clear();

    QSettings s;
    int size = s.beginReadArray(QStringLiteral("Endpoints"));
    for (int i = 0; i < size; ++i)
    {
        s.setArrayIndex(i);
        EndpointConfig c;
        c.load(s);
        configs_ << c;
    }
    s.endArray();

    endResetModel();

    return true;
}

bool EndpointConfigModel::save()
{
    QSettings s;

    s.beginWriteArray(QStringLiteral("Endpoints"), configs_.size());
    for (int i = 0; i < configs_.size(); ++i)
    {
        s.setArrayIndex(i);
        configs_[i].save(s);
    }
    s.endArray();

    return true;
}
