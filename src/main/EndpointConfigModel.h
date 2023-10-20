// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "EndpointConfig.h"
#include <QAbstractTableModel>

class EndpointConfigModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit EndpointConfigModel(QObject* parent = nullptr);
    ~EndpointConfigModel() override;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int addConfig(const EndpointConfig& config);
    void removeConfig(int index);

    bool load();
    bool save();

private:
    QList<EndpointConfig> configs_;
};
