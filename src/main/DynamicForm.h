// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

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

    enum class Filter
    {
        All,
        Modified,
    };

public:
    explicit DynamicForm(QWidget* parent = {});

    void clear();
    void reset(const QList<Field>& fields);

    QHash<QString, QString> extractFormData(Filter filter = Filter::All);
    void fillFormData(const QHash<QString, QString>& formData);

signals:

private:
    QFormLayout* layout_;
};
