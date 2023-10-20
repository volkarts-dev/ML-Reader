// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractTableModel>

class DataModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(bool firstRowHeader READ firstRowHeader WRITE setFirstRowHeader NOTIFY firstRowHeaderChanged)

public:
    explicit DataModel(QObject* parent = nullptr);

    const QList<QStringList>& modelData() const { return data_; }
    void setModelData(const QList<QStringList>& data, bool headerDetect = true);
    bool firstRowHeader() const { return firstRowHeader_; }
    int detectedPidColumn() const { return detectedPidColumn_; }
    bool hasData() const;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void setFirstRowHeader(bool firstRowHeader);

signals:
    void firstRowHeaderChanged(bool firstRawHeader);

private:
    bool detectFirstRowHeader(const QList<QStringList>& data);

private:
    QList<QStringList> data_;
    bool firstRowHeader_{true};
    int detectedPidColumn_{-1};
};
