// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "MainInterface.h"
#include <QMainWindow>

class DataModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public MainInterface
{
    Q_OBJECT

public:
    static MainWindow* find(QWidget* base);

public:
    explicit MainWindow(QWidget* parent = {});
    ~MainWindow() override;

    void showStatusMessage(const QString& message, int timeout = 0) override;
    void openPage(Page page, const QVariant& openData) override;

private slots:
    void onActionEndpointConfigEdit();
    void onActionQuitTriggerd();
    void onShowLoaderPageTriggered();
    void onShowQueryPageTriggered();
    void onShowEditPageTriggered();
    void onFunctionStackCurrentChanged(int index);

private:
    bool setup();
    void loadMainWindowState();
    void saveMainWindowState();

private:
    Ui::MainWindow* ui;

    Q_DISABLE_COPY_MOVE(MainWindow)
};
