// Copyright 2023, Daniel Volk <mail@volkarts.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "EndpointConfigEditDlg.h"
#include "Tools.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QThreadPool>
#include <QSettings>

MainWindow* MainWindow::find(QWidget* base)
{
    while (base)
    {
        if (auto* mainWindow = qobject_cast<MainWindow*>(base))
            return mainWindow;
        base = base->parentWidget();
    }
    return nullptr;
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow{}}
{
    ui->setupUi(this);
    setup();
}

MainWindow::~MainWindow()
{
    saveMainWindowState();

    delete ui;
}

bool MainWindow::setup()
{
    ui->loaderPage->setMainInterface(this);
    connect(ui->loaderPage, &LoaderPage::selectedEndpointChanged, this, &MainWindow::onSelectedEndpointChanged);
    ui->queryPage->setMainInterface(this);
    connect(ui->queryPage, &QueryPage::selectedEndpointChanged, this, &MainWindow::onSelectedEndpointChanged);
    ui->editorPage->setMainInterface(this);
    connect(ui->editorPage, &EditorPage::selectedEndpointChanged, this, &MainWindow::onSelectedEndpointChanged);

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

    connect(ui->actionShowLoaderPage, &QAction::triggered, this, &MainWindow::onShowLoaderPageTriggered);
    connect(ui->actionShowQueryPage, &QAction::triggered, this, &MainWindow::onShowQueryPageTriggered);
    connect(ui->actionShowEditorPage, &QAction::triggered, this, &MainWindow::onShowEditPageTriggered);

    connect(ui->functionStack, &QTabWidget::currentChanged, this, &MainWindow::onFunctionStackCurrentChanged);

    loadMainWindowState();

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
    QSettings s;

    restoreGeometry(s.value("Window/Geometry").toByteArray());
    restoreState(s.value("Window/State").toByteArray());

    int functionIndex = indexClamp(s.value("Window/FunctionPage").toInt(), ui->functionStack->count() - 1, 0);
    ui->functionStack->setCurrentIndex(functionIndex);
    onFunctionStackCurrentChanged(functionIndex);
}

void MainWindow::saveMainWindowState()
{
    QSettings s;

    s.setValue("Window/Geometry", saveGeometry());
    s.setValue("Window/State", saveState());
    s.setValue("Window/FunctionPage", ui->functionStack->currentIndex());
}

void MainWindow::onActionEndpointConfigEdit()
{
    EndpointConfigEditDlg dlg{this};
    dlg.exec();

    ui->loaderPage->handleEndpointConfigChanged();
    ui->queryPage->handleEndpointConfigChanged();
    ui->editorPage->handleEndpointConfigChanged();
}

void MainWindow::onActionQuitTriggerd()
{
    QApplication::quit();
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
    ui->loaderPage->setSelectedEndpoint(index);
    ui->queryPage->setSelectedEndpoint(index);
    ui->editorPage->setSelectedEndpoint(index);
}
