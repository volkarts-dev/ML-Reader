// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QMainWindow>

class DataModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* find(QWidget* base);

public:
    explicit MainWindow(QWidget* parent = {});
    ~MainWindow() override;

    void showStatusMessage(const QString& message, int timeout = 0);

private slots:
    void onActionEndpointConfigEdit();
    void onActionQuitTriggerd();

private:
    bool setup();
    void loadMainWindowState();
    void saveMainWindowState();

private:
    Ui::MainWindow* ui;
};
