// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: <LICENSE>

#pragma once

#include <QWidget>

namespace Ui {
class EndpointSelector;
}

class EndpointSelector : public QWidget
{
    Q_OBJECT

public:
    explicit EndpointSelector(QWidget* parent = {});
    ~EndpointSelector() override;

    int selectedEndpoint();
    void setSelectedEndpoint(int index);

    QString currentApiKey() const;
    void saveApiKey();

signals:
    void selectedEnpointChanged(int index);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onEndpointSelectorChanged(int index);
    void onPasswordLoaded(bool result, const QUuid& uuid, const QString& password);
    void onSaveApiKeyChanged(bool state);


private:
    void setup();
    QUuid currentEndpointUuid();

private:
    Ui::EndpointSelector* ui;

    Q_DISABLE_COPY_MOVE(EndpointSelector)
};