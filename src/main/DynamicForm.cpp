// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "DynamicForm.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

DynamicForm::Field::Field(const QString& name) :
    name_{name},
    caption_{name}
{
}

DynamicForm::Field::Field(QString name, QString caption) :
    name_{std::move(name)},
    caption_{std::move(caption)}
{
}

// ********************************************************

DynamicForm::DynamicForm(QWidget* parent) :
    QWidget{parent},
    layout_{new QFormLayout{this}}
{
    setLayout(layout_);
}

void DynamicForm::clear()
{
    while (layout_->rowCount() > 0)
    {
        layout_->removeRow(layout_->rowCount() - 1);
    }
}

void DynamicForm::reset(const QList<Field>& fields)
{
    clear();

    for (const auto& field : fields)
    {
        auto caption = new QLabel{this};
        caption->setObjectName(field.name() + "_caption");
        caption->setText(field.caption());

        auto input = new QLineEdit{this};
        input->setObjectName(field.name());

        layout_->addRow(caption, input);
    }
}
