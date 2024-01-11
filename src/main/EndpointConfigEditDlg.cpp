// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "EndpointConfigEditDlg.h"
#include "ui_EndpointConfigEditDlg.h"

#include "Application.h"
#include "EndpointConfigItemDelegate.h"
#include "EndpointConfigModel.h"
#include "Tools.h"
#include <QDataWidgetMapper>

EndpointConfigEditDlg::EndpointConfigEditDlg(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::EndpointConfigEditDlg),
    mapper_{new QDataWidgetMapper{this}},
    itemDelegate_{new EndpointConfigItemDelegate{this}}
{
    ui->setupUi(this);
    setup();
}

EndpointConfigEditDlg::~EndpointConfigEditDlg()
{
    delete ui;
}

void EndpointConfigEditDlg::accept()
{
    app()->endpointConfigModel()->save();

    QDialog::accept();
}

void EndpointConfigEditDlg::setup()
{
    ui->configsList->setModel(app()->endpointConfigModel());
    ui->configsList->setModelColumn(toInt(EndpointConfig::Field::Name));

    ui->detailsContainer->setEnabled(false);
    mapper_->setItemDelegate(itemDelegate_);

    mapper_->setModel(app()->endpointConfigModel());
    mapper_->addMapping(ui->name, static_cast<int>(EndpointConfig::Field::Name));
    mapper_->addMapping(ui->baseURL, static_cast<int>(EndpointConfig::Field::BaseURL));
    mapper_->addMapping(ui->apiVersion, static_cast<int>(EndpointConfig::Field::ApiVersion));
    mapper_->addMapping(ui->fields, static_cast<int>(EndpointConfig::Field::Fields));

    connect(ui->configsList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EndpointConfigEditDlg::onSelectionChanged);

    connect(ui->addBtn, &QToolButton::clicked, this, &EndpointConfigEditDlg::onAddButtonClicked);
    connect(ui->removeBtn, &QToolButton::clicked, this, &EndpointConfigEditDlg::onRemoveButtonClicked);

    onSelectionChanged({}, {});
}

void EndpointConfigEditDlg::onAddButtonClicked()
{
    int newIndex = app()->endpointConfigModel()->addConfig({tr("<New Config>")});
    ui->configsList->selectionModel()->select(ui->configsList->model()->index(newIndex, 0),
                                              QItemSelectionModel::ClearAndSelect);
}

void EndpointConfigEditDlg::onRemoveButtonClicked()
{
    auto selection = ui->configsList->selectionModel()->selection();
    Q_ASSERT(!selection.isEmpty());

    app()->endpointConfigModel()->removeConfig(selection[0].top());
}

void EndpointConfigEditDlg::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    ui->detailsContainer->setEnabled(!selected.empty());
    ui->removeBtn->setEnabled(!selected.empty());

    if (!selected.empty())
        mapper_->setCurrentModelIndex(selected[0].topLeft());
    else
        mapper_->setCurrentModelIndex({});
}
