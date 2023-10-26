// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#include "EndpointSelector.h"
#include "ui_EndpointSelector.h"

#include "Application.h"
#include "EndpointConfigModel.h"
#include "PasswordStore.h"
#include "Tools.h"
#include <QUuid>

EndpointSelector::EndpointSelector(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EndpointSelector)
{
    ui->setupUi(this);
    setup();
}

EndpointSelector::~EndpointSelector()
{
    delete ui;
}

void EndpointSelector::setup()
{
    ui->endpointSelector->setModel(app()->endpointConfigModel());
    ui->endpointSelector->setModelColumn(toInt(EndpointConfig::Field::Name));
    ui->endpointSelector->setCurrentIndex(-1);

    connect(ui->endpointSelector, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &EndpointSelector::onEndpointSelectorChanged);

    connect(ui->apiKey, &QLineEdit::editingFinished, this, &EndpointSelector::onApiKeyChanged);
    connect(ui->saveApiKey, &QCheckBox::stateChanged, this, &EndpointSelector::onSaveApiKeyChanged);

    connect(app()->passwordStore(), &PasswordStore::passwordLoaded, this, &EndpointSelector::onPasswordLoaded);
}

QUuid EndpointSelector::currentEndpointUuid()
{
    auto index = ui->endpointSelector->currentIndex();
    if (index == -1)
        return {};
    auto* model = app()->endpointConfigModel();
    auto uuidData = model->data(model->index(index, toInt(EndpointConfig::Field::Uuid)), Qt::DisplayRole);
    return uuidData.toUuid();
}

int EndpointSelector::selectedEndpoint()
{
    return ui->endpointSelector->currentIndex();
}

void EndpointSelector::setSelectedEndpoint(int index)
{
    ui->endpointSelector->setCurrentIndex(index);
}

QString EndpointSelector::currentApiKey() const
{
    return ui->apiKey->text();
}

QStringList EndpointSelector::currentFieldList() const
{
    auto index = ui->endpointSelector->currentIndex();
    if (index == -1)
        return {};
    auto model = app()->endpointConfigModel();
    auto fieldsData = model->data(model->index(index, toInt(EndpointConfig::Field::Fields)), Qt::DisplayRole);
    return fieldsData.toStringList();
}

void EndpointSelector::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);

    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

        default:
            break;
    }
}

void EndpointSelector::onEndpointSelectorChanged(int index)
{
    endpointChanging_ = true;

    auto model = app()->endpointConfigModel();
    {
        const auto mi = model->index(index, toInt(EndpointConfig::Field::BaseURL));
        ui->baseURL->setText(model->data(mi, Qt::DisplayRole).toString());
    }
    {
        const auto mi = model->index(index, toInt(EndpointConfig::Field::SaveApiKey));
        ui->saveApiKey->setCheckState(model->data(mi, Qt::DisplayRole).toBool() ? Qt::Checked : Qt::Unchecked);
    }

    endpointChanging_ = false;

    ui->apiKey->setText({});
    if (ui->saveApiKey->isChecked())
        app()->passwordStore()->loadPassword(currentEndpointUuid());


    emit selectedEnpointChanged(index);
}

void EndpointSelector::onPasswordLoaded(bool result, const QUuid& uuid, const QString& password)
{
    Q_UNUSED(result)

    if (uuid != currentEndpointUuid())
        return;

    ui->apiKey->setText(password);
}

void EndpointSelector::onApiKeyChanged()
{
    if (ui->saveApiKey->isChecked())
        app()->passwordStore()->savePassword(currentEndpointUuid(), ui->apiKey->text());
}

void EndpointSelector::onSaveApiKeyChanged(int state)
{
    if (endpointChanging_)
        return;

    if (state == Qt::Checked)
        app()->passwordStore()->savePassword(currentEndpointUuid(), ui->apiKey->text());
    else
        app()->passwordStore()->removePassword(currentEndpointUuid());

    auto model = app()->endpointConfigModel();
    const auto mi = model->index(ui->endpointSelector->currentIndex(), toInt(EndpointConfig::Field::SaveApiKey));
    model->setData(mi, state == Qt::Checked, Qt::EditRole);
    app()->endpointConfigModel()->save();
}
