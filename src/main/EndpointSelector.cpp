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

    connect(ui->saveApiKey, &QCheckBox::stateChanged, this, &EndpointSelector::onSaveApiKeyChanged);

    connect(app()->passwordStore(), &PasswordStore::passwordLoaded, this, &EndpointSelector::onPasswordLoaded);
}

QUuid EndpointSelector::currentEndpointUuid()
{
    auto index = ui->endpointSelector->currentIndex();
    if (index == -1)
        return {};

    auto* model = app()->endpointConfigModel();
    auto uuidData = model->data(model->index(index, toInt(EndpointConfig::Field::Uuid)),
                            Qt::DisplayRole);
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

void EndpointSelector::saveApiKey()
{
    if (ui->apiKey->isModified())
    {
        if (ui->saveApiKey->isChecked())
            app()->passwordStore()->savePassword(currentEndpointUuid(), ui->apiKey->text());
        ui->apiKey->setModified(false);
    }
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
    auto model = app()->endpointConfigModel();
    auto modelIndex = model->index(index, toInt(EndpointConfig::Field::BaseURL));
    ui->baseURL->setText(model->data(modelIndex, Qt::DisplayRole).toString());

    app()->passwordStore()->loadPassword(currentEndpointUuid());

    emit selectedEnpointChanged(index);
}

void EndpointSelector::onPasswordLoaded(bool result, const QUuid& uuid, const QString& password)
{
    if (uuid != currentEndpointUuid())
        return;

    ui->apiKey->setText(password);
}

void EndpointSelector::onSaveApiKeyChanged(bool state)
{
    if (!state)
        app()->passwordStore()->removePassword(currentEndpointUuid());
    else
        ui->apiKey->setModified(true);
}
