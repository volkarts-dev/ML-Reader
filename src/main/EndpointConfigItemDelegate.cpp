// Copyright 2024, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EndpointConfigItemDelegate.h"

#include "EndpointConfig.h"
#include <QPlainTextEdit>

EndpointConfigItemDelegate::EndpointConfigItemDelegate(QObject* parent) :
    QStyledItemDelegate{parent}
{
}

void EndpointConfigItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() != static_cast<int>(EndpointConfig::Field::Fields))
    {
        QStyledItemDelegate::setEditorData(editor, index);
        return;
    }

    auto* textEdit = qobject_cast<QPlainTextEdit*>(editor);
    Q_ASSERT(textEdit);

    const auto list = index.data().value<QStringList>();

    textEdit->setPlainText(list.join(QLatin1String(", ")));
}

void EndpointConfigItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() != static_cast<int>(EndpointConfig::Field::Fields))
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    auto* textEdit = qobject_cast<QPlainTextEdit*>(editor);
    Q_ASSERT(textEdit);

    const auto plainString = textEdit->toPlainText();

    QRegularExpression expr{QStringLiteral("[,;\\s]+")};
    const auto fields = plainString.split(expr, Qt::SkipEmptyParts);

    model->setData(index, fields);
}
