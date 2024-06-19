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
#include <QScrollBar>
#include <QThreadPool>

namespace {

const auto CfgWindowGeometry = QStringLiteral("Window/Main/Geometry");
const auto CfgWindowState = QStringLiteral("Window/Main/State");
const auto CfgWindowSelectedEndpoint = QStringLiteral("Window/Main/SelectedEndpoint");
const auto CfgWindowFunctionPage = QStringLiteral("Window/Main/FunctionPage");
const auto CfgWindowMainSplitter= QStringLiteral("Window/Main/MainSplitter");

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

    ui->mainSplitter->setStretchFactor(ui->mainSplitter->indexOf(ui->functionStack), 10);
    ui->mainSplitter->setStretchFactor(ui->mainSplitter->indexOf(ui->logOutput), 1);

    connect(ui->endpointSelector, &EndpointSelector::selectedEndpointChanged,
            this, &MainWindow::onSelectedEndpointChanged);
    connect(this, &MainWindow::endpointConfigChanged, ui->endpointSelector, &EndpointSelector::onEndpointConfigChanged);

    ui->functionStack->setTabIcon(toInt(Page::Loader), QIcon::fromTheme(QStringLiteral("download")));
    ui->functionStack->setTabIcon(toInt(Page::Query), QIcon::fromTheme(QStringLiteral("system-search")));
    ui->functionStack->setTabIcon(toInt(Page::Editor), QIcon::fromTheme(QStringLiteral("document-edit")));

    ui->actionEndpointConfigEdit->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    ui->actionQuit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    ui->actionShowLoaderPage->setIcon(QIcon::fromTheme(QStringLiteral("download")));
    ui->actionShowQueryPage->setIcon(QIcon::fromTheme(QStringLiteral("system-search")));
    ui->actionShowEditorPage->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));

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

void MainWindow::logMessage(QtMsgType type, const QString& message)
{
    QString string;

    switch (type)
    {
        case QtDebugMsg:
            string += QStringLiteral("<span style='color: gray'>[DEBUG] ");
            break;
        case QtInfoMsg:
            string += QStringLiteral("<span style=''>[INFO ");
            break;
        case QtWarningMsg:
            string += QStringLiteral("<span style='color: yellow'>[WARN] ");
            break;
        case QtCriticalMsg:
            string += QStringLiteral("<span style='color: red'>[ERROR] ");
            break;
        case QtFatalMsg:
            string += QStringLiteral("<span style='color: purple'>[FATAL] ");
            break;
    }

    string += message + QStringLiteral("</span>");

    auto* scrollBar = ui->logOutput->verticalScrollBar();
    bool scrollbarAtBottom  = (scrollBar->value() >= (scrollBar->maximum() - 10));

    ui->logOutput->appendHtml(string);

    if (scrollbarAtBottom)
        scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::loadMainWindowState()
{
    UserSettings s;

    restoreGeometry(s.value(CfgWindowGeometry).toByteArray());
    restoreState(s.value(CfgWindowState).toByteArray());

    restoreSplitterState(ui->mainSplitter, s.value(CfgWindowMainSplitter).toByteArray());

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
    s.setValue(CfgWindowMainSplitter, saveSplitterState(ui->mainSplitter));
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
                           time.toString(QStringLiteral("yyyy")),
                           APPLICATION_VERSION,
                           GIT_VERSION,
                           time.toString(QStringLiteral("yyyy-MM-dd hh:mm"))
                           ));
}

void MainWindow::onShowLoaderPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Loader));
}

void MainWindow::onShowQueryPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Query));
}

void MainWindow::onShowEditPageTriggered()
{
    ui->functionStack->setCurrentIndex(toInt(Page::Editor));
}

void MainWindow::onFunctionStackCurrentChanged(int index)
{
    ui->actionShowLoaderPage->setChecked(index == toInt(Page::Loader));
    ui->actionShowQueryPage->setChecked(index == toInt(Page::Query));
    ui->actionShowEditorPage->setChecked(index == toInt(Page::Editor));

    if (index == toInt(Page::Loader))
    {
        setWindowTitle(tr("Loader - ML Client"));
    }
    else if (index == toInt(Page::Query))
    {
        setWindowTitle(tr("Query - ML Client"));
    }
    else if (index == toInt(Page::Editor))
    {
        setWindowTitle(tr("Editor - ML Client"));
    }
}

void MainWindow::onSelectedEndpointChanged(int index)
{
    emit selectedEndpointChanged(index);
}
