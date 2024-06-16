// Copyright 2024, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QStyledItemDelegate>

class EndpointConfigItemDelegate : public QStyledItemDelegate
{
public:
    EndpointConfigItemDelegate(QObject *parent = nullptr);

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};
