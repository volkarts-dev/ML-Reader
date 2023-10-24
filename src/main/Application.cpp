// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "Application.h"

#include "Configuration.h"
#include "MainWindow.h"
#include "EndpointConfigModel.h"
#include "PasswordStore.h"
#include <QThreadPool>

namespace {

Application* gApp = nullptr;

} // namespace

Application* app()
{
    return gApp;
}

Application::Application(int& argc, char** argv) :
    QApplication(argc, argv)
{
    Q_ASSERT(!gApp);
    gApp = this;

    qSetMessagePattern("%{category} [%{type}] %{message}");

    Application::setApplicationName(QStringLiteral("mlreader"));
    Application::setOrganizationName(QStringLiteral("volkarts.com"));
    Application::setOrganizationDomain(QStringLiteral("volkarts.com"));

    if (QIcon::themeName().isEmpty())
        QIcon::setThemeName(QStringLiteral("breeze"));
}

Application::~Application()
{
    while (runningJobs_ > 0)
    {
        QThread::sleep(100);
    }
}

bool Application::initialize()
{
    configuration_.reset(new Configuration{});

    endpointConfigModel_.reset(new EndpointConfigModel{});
    if (!endpointConfigModel_->load())
        return false;

    passwordStore_.reset(new PasswordStore{});

    mainWindow_.reset(new MainWindow());
    mainWindow_->setVisible(true);

    return true;
}

void Application::runJob(const std::function<void()>& runnable)
{
    ++runningJobs_;
    QThreadPool::globalInstance()->start([this, runnable]()
    {
        runnable();
        --runningJobs_;
    });
}
