// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Application.h"
#include "EndpointConfigEditDlg.h"
#include "EndpointConfigModel.h"
#include "Tools.h"
#include "Version.h"
#include "UserSettings.h"
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QThreadPool>

namespace {

const auto CfgWindowGeometry = QString("Window/Main/Geometry");
const auto CfgWindowState = QString("Window/Main/State");
const auto CfgWindowSelectedEndpoint = QString("Window/Main/SelectedEndpoint");
const auto CfgWindowFunctionPage = QString("Window/Main/FunctionPage");

} // namespace

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow{}}
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    saveMainWindowState();

    delete ui;
}

EndpointSelector* MainWindow::endpointSelector() const
{
    return ui->endpointSelector;
}

bool MainWindow::initialize()
{
    ui->loaderPage->initialize(this);
    ui->queryPage->initialize(this);
    ui->editorPage->initialize(this);

    connect(ui->endpointSelector, &EndpointSelector::selectedEndpointChanged,
            this, &MainWindow::onSelectedEndpointChanged);
    connect(this, &MainWindow::endpointConfigChanged, ui->endpointSelector, &EndpointSelector::onEndpointConfigChanged);

    ui->functionStack->setTabIcon(toInt(Page::Loader), QIcon::fromTheme("download"));
    ui->functionStack->setTabIcon(toInt(Page::Query), QIcon::fromTheme("system-search"));
    ui->functionStack->setTabIcon(toInt(Page::Editor), QIcon::fromTheme("document-edit"));

    ui->actionEndpointConfigEdit->setIcon(QIcon::fromTheme("configure"));
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    ui->actionShowLoaderPage->setIcon(QIcon::fromTheme("download"));
    ui->actionShowQueryPage->setIcon(QIcon::fromTheme("system-search"));
    ui->actionShowEditorPage->setIcon(QIcon::fromTheme("document-edit"));

    connect(ui->actionEndpointConfigEdit, &QAction::triggered, this, &MainWindow::onActionEndpointConfigEdit);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::onActionQuitTriggerd);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggerd);

    connect(ui->actionShowLoaderPage, &QAction::triggered, this, &MainWindow::onShowLoaderPageTriggered);
    connect(ui->actionShowQueryPage, &QAction::triggered, this, &MainWindow::onShowQueryPageTriggered);
    connect(ui->actionShowEditorPage, &QAction::triggered, this, &MainWindow::onShowEditPageTriggered);

    connect(ui->functionStack, &QTabWidget::currentChanged, this, &MainWindow::onFunctionStackCurrentChanged);

    loadMainWindowState();

    updateUi();
    setVisible(true);

    return true;
}

void MainWindow::showStatusMessage(const QString& message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
}

void MainWindow::openPage(Page page, const QVariant& openData)
{
    switch (page)
    {
        using enum Page;

        case Loader:
            ui->actionShowLoaderPage->trigger();
            break;

        case Query:
            ui->actionShowQueryPage->trigger();
            break;

        case Editor:
            ui->actionShowEditorPage->trigger();
            if (!openData.isNull())
                ui->editorPage->startEditing(openData.toString());
            break;
    }
}

void MainWindow::loadMainWindowState()
{
    UserSettings s;

    restoreGeometry(s.value(CfgWindowGeometry).toByteArray());
    restoreState(s.value(CfgWindowState).toByteArray());

    ui->endpointSelector->setSelectedEndpoint(
                indexClamp(s.value(CfgWindowSelectedEndpoint).toInt(),
                           app()->endpointConfigModel()->rowCount() - 1));

    int functionIndex = indexClamp(s.value(CfgWindowFunctionPage).toInt(), ui->functionStack->count() - 1, 0);
    if (ui->functionStack->currentIndex() != functionIndex)
        ui->functionStack->setCurrentIndex(functionIndex);
    else
        onFunctionStackCurrentChanged(functionIndex);
}

void MainWindow::saveMainWindowState()
{
    UserSettings s;

    s.setValue(CfgWindowGeometry, saveGeometry());
    s.setValue(CfgWindowState, saveState());
    s.setValue(CfgWindowSelectedEndpoint, ui->endpointSelector->selectedEndpoint());
    s.setValue(CfgWindowFunctionPage, ui->functionStack->currentIndex());
}

void MainWindow::updateUi()
{
    const auto model = app()->endpointConfigModel();
    const auto emptyConfig = model->rowCount() == 0;

    ui->centralwidget->setEnabled(!emptyConfig);
    ui->menuFunction->setEnabled(!emptyConfig);
}

void MainWindow::onActionEndpointConfigEdit()
{
    EndpointConfigEditDlg dlg{this};
    dlg.exec();

    updateUi();

    emit endpointConfigChanged();
}

void MainWindow::onActionQuitTriggerd()
{
    QApplication::quit();
}

void MainWindow::onActionAboutTriggerd()
{
    const auto time = QDateTime::fromSecsSinceEpoch(GIT_TIMESTAMP);

    QMessageBox::about(this, tr("About ML-Reader"), tr(
                           "ML-Reader\n"
                           "Copyright: 2023-%1 Daniel Volk\n"
                           "Version: %2-%3\n"
                           "Date: %4"
                           ).arg(
                           time.toString("yyyy"),
                           APPLICATION_VERSION,
                           GIT_VERSION,
                           time.toString("yyyy-MM-dd hh:mm")
                           ));
}

void MainWindow::onShowLoaderPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Loader));
    setWindowTitle(tr("Loader - ML Client"));
}

void MainWindow::onShowQueryPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Query));
    setWindowTitle(tr("Query - ML Client"));
}

void MainWindow::onShowEditPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Editor));
    setWindowTitle(tr("Editor - ML Client"));
}

void MainWindow::onFunctionStackCurrentChanged(int index)
{
    ui->actionShowLoaderPage->setChecked(index == toInt(Page::Loader));
    ui->actionShowQueryPage->setChecked(index == toInt(Page::Query));
    ui->actionShowEditorPage->setChecked(index == toInt(Page::Editor));
}

void MainWindow::onSelectedEndpointChanged(int index)
{
    emit selectedEndpointChanged(index);
}
