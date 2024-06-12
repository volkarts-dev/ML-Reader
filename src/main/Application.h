// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QApplication>

class MainWindow;
class EndpointConfigModel;
class PasswordStore;

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application() override;

    bool initialize();

    EndpointConfigModel* endpointConfigModel() const { return endpointConfigModel_.get(); }
    PasswordStore* passwordStore() const { return passwordStore_.get(); }
    MainWindow* mainWindow() const { return mainWindow_.get(); }

    void runJob(const std::function<void()>& runnable);

private:
    QScopedPointer<EndpointConfigModel> endpointConfigModel_;
    QScopedPointer<PasswordStore> passwordStore_;
    QScopedPointer<MainWindow> mainWindow_;
    QAtomicInt runningJobs_{};

    Q_DISABLE_COPY_MOVE(Application)
};

Application* app();
