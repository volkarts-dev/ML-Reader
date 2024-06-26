// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QMainWindow>

class DataModel;
class EndpointSelector;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class Page
    {
        Loader,
        Query,
        Editor,
    };

public:
    explicit MainWindow(QWidget* parent = {});
    ~MainWindow() override;

    bool initialize();

    EndpointSelector* endpointSelector() const;

    void showStatusMessage(const QString& message, int timeout = 0);
    void openPage(Page page, const QVariant& openData);

public slots:
    void logMessage(QtMsgType type, const QString& message);

signals:
    void endpointConfigChanged();
    void selectedEndpointChanged(int index);

private slots:
    void onActionEndpointConfigEdit();
    void onActionQuitTriggerd();
    void onActionAboutTriggerd();
    void onShowLoaderPageTriggered();
    void onShowQueryPageTriggered();
    void onShowEditPageTriggered();
    void onFunctionStackCurrentChanged(int index);
    void onSelectedEndpointChanged(int index);

private:
    void loadMainWindowState();
    void saveMainWindowState();
    void updateUi();

private:
    Ui::MainWindow* ui;

    Q_DISABLE_COPY_MOVE(MainWindow)
};
