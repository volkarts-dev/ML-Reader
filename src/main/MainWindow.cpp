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
    ui->queryPage->setMainInterface(this);
    ui->editorPage->setMainInterface(this);

    ui->functionStack->setTabIcon(toInt(Page::Loader), QIcon::fromTheme("download"));
    ui->functionStack->setTabIcon(toInt(Page::Query), QIcon::fromTheme("system-search"));
    ui->functionStack->setTabIcon(toInt(Page::Editor), QIcon::fromTheme("document-edit"));
    ui->functionStack->setCurrentIndex(-1);

    ui->actionEndpointConfigEdit->setIcon(QIcon::fromTheme("configure"));
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    ui->actionShowLoaderPage->setIcon(QIcon::fromTheme("download"));
    ui->actionShowQueryPage->setIcon(QIcon::fromTheme("system-search"));
    ui->actionShowEditorPage->setIcon(QIcon::fromTheme("document-edit"));
    loadMainWindowState();

    connect(ui->actionEndpointConfigEdit, &QAction::triggered, this, &MainWindow::onActionEndpointConfigEdit);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::onActionQuitTriggerd);

    connect(ui->actionShowLoaderPage, &QAction::triggered, this, &MainWindow::onShowLoaderPageTriggered);
    connect(ui->actionShowQueryPage, &QAction::triggered, this, &MainWindow::onShowQueryPageTriggered);
    connect(ui->actionShowEditorPage, &QAction::triggered, this, &MainWindow::onShowEditPageTriggered);

    connect(ui->functionStack, &QTabWidget::currentChanged, this, &MainWindow::onFunctionStackCurrentChanged);

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
    ui->functionStack->setCurrentIndex(indexClamp(s.value("Window/FunctionPage").toInt(),
                                                         ui->functionStack->count() - 1, 0));
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
}

void MainWindow::onActionQuitTriggerd()
{
    QApplication::quit();
}

void MainWindow::onShowLoaderPageTriggered()
{
    ui->functionStack->setCurrentIndex(0);
    setWindowTitle(tr("Loader - ML Client"));
}

void MainWindow::onShowQueryPageTriggered()
{
    ui->functionStack->setCurrentIndex(1);
    setWindowTitle(tr("Query - ML Client"));
}

void MainWindow::onShowEditPageTriggered()
{
    ui->functionStack->setCurrentIndex(2);
    setWindowTitle(tr("Edit - ML Client"));
}

void MainWindow::onFunctionStackCurrentChanged(int index)
{
    ui->actionShowLoaderPage->setChecked(index == 0);
    ui->actionShowQueryPage->setChecked(index == 1);
    ui->actionShowEditorPage->setChecked(index == 2);
}
