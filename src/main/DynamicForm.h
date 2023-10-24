// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include <QWidget>

class QFormLayout;

class DynamicForm : public QWidget
{
    Q_OBJECT

public:
    class Field
    {
    public:
        Field(const QString& name);
        Field(QString name, QString caption);

        QString name() const { return name_; }
        QString caption() const { return caption_; }

    private:
        QString name_;
        QString caption_;
    };

public:
    explicit DynamicForm(QWidget* parent = {});

    void clear();
    void reset(const QList<Field>& fields);

    QHash<QString, QString> extractFormData();

signals:

private:
    QFormLayout* layout_;
};