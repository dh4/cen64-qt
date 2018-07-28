/***
 * Copyright (c) 2013, Dan Hasting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the organization nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

#include "mainwindow.h"

#include "global.h"
#include "common.h"

#include "dialogs/aboutdialog.h"
#include "dialogs/downloaddialog.h"
#include "dialogs/logdialog.h"
#include "dialogs/settingsdialog.h"
#include "dialogs/v64converter.h"

#include "emulation/emulatorhandler.h"

#include "roms/romcollection.h"
#include "roms/thegamesdbscraper.h"

#include "views/gridview.h"
#include "views/listview.h"
#include "views/tableview.h"
#include "views/ddview.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(AppName);
    setWindowIcon(QIcon(":/images/"+ParentNameLower+".png"));
    installEventFilter(this);

    emulation = new EmulatorHandler(this);
    romCollection = new RomCollection(QStringList() << "*.z64" << "*.n64" << "*.zip" << "*.ndd",
                                      QStringList() << SETTINGS.value("Paths/roms","").toString().split("|"),
                                      this);
    createMenu();
    createRomView();

    connect(emulation, SIGNAL(started()), this, SLOT(disableButtons()));
    connect(emulation, SIGNAL(finished()), this, SLOT(enableButtons()));
    connect(emulation, SIGNAL(showLog()), this, SLOT(openLog()));
    connect(emulation, SIGNAL(statusUpdate(QString, int)), this, SLOT(updateStatusBar(QString, int)));

    connect(romCollection, SIGNAL(updateStarted(bool)), this, SLOT(disableViews(bool)));
    connect(romCollection, SIGNAL(romAdded(Rom*, int)), this, SLOT(addToView(Rom*, int)));
    connect(romCollection, SIGNAL(ddRomAdded(Rom*)), ddView, SLOT(addTo64DDView(Rom*)));
    connect(romCollection, SIGNAL(updateEnded(int, bool)), this, SLOT(enableViews(int, bool)));

    romCollection->cachedRoms(false, true);


    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    statusBar = new QStatusBar;

    if (SETTINGS.value("View/statusbar", "").toString() == "")
        statusBar->hide();
    if (SETTINGS.value("View/fullscreen", "").toString() == "true")
        updateFullScreenMode();

    QString ddipl = SETTINGS.value("Paths/ddiplrom", "").toString();
    QString ddEnabled = SETTINGS.value("Emulation/64dd", "").toString();
    QString currentView = SETTINGS.value("View/layout", "none").toString();

    if (ddipl != "" && ddEnabled == "true" && currentView != "none")
        ddView->setHidden(false);

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setMenuBar(menuBar);

    mainLayout->addWidget(viewSplitter);

    mainLayout->addWidget(statusBar);
    mainLayout->setMargin(0);

    mainWidget->setLayout(mainLayout);
    mainWidget->setMinimumSize(300, 200);
}


void MainWindow::addToView(Rom *currentRom, int count)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        tableView->addToTableView(currentRom);
    else if (visibleLayout == "grid")
        gridView->addToGridView(currentRom, count, ddAction->isChecked());
    else if (visibleLayout == "list")
        listView->addToListView(currentRom, count, ddAction->isChecked());
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    SETTINGS.setValue("Geometry/windowx", geometry().x());
    SETTINGS.setValue("Geometry/windowy", geometry().y());
    SETTINGS.setValue("Geometry/width", geometry().width());
    SETTINGS.setValue("Geometry/height", geometry().height());
    if (isMaximized())
        SETTINGS.setValue("Geometry/maximized", true);
    else
        SETTINGS.setValue("Geometry/maximized", "");

    tableView->saveColumnWidths();

    QStringList sizes;
    foreach(int size, viewSplitter->sizes())
        sizes << QString::number(size);
    SETTINGS.setValue("View/64ddsize", sizes.join("|"));

    event->accept();
}


void MainWindow::createMenu()
{
    menuBar = new QMenuBar(this);


    //File
    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open ROM..."));
    fileMenu->addSeparator();
    convertAction = fileMenu->addAction(tr("&Convert V64/n64..."));
    refreshAction = fileMenu->addAction(tr("&Refresh List"));
    downloadAction = fileMenu->addAction(tr("&Download/Update Info..."));
    deleteAction = fileMenu->addAction(tr("D&elete Current Info..."));
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    fileMenu->addSeparator();
#endif
    quitAction = fileMenu->addAction(tr("&Quit"));

    openAction->setIcon(QIcon::fromTheme("document-open"));
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));

    downloadAction->setEnabled(false);
    deleteAction->setEnabled(false);

    menuBar->addMenu(fileMenu);

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(convertAction, SIGNAL(triggered()), this, SLOT(openConverter()));
    connect(refreshAction, SIGNAL(triggered()), romCollection, SLOT(addRoms()));
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(openDeleteDialog()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));


    //Emulation
    emulationMenu = new QMenu(tr("&Emulation"), this);
    startAction = emulationMenu->addAction(tr("&Start"));
    stopAction = emulationMenu->addAction(tr("St&op"));
    emulationMenu->addSeparator();
    ddAction = emulationMenu->addAction(tr("Enable 64DD"));
    emulationMenu->addSeparator();
    logAction = emulationMenu->addAction(tr("View Log..."));

    startAction->setIcon(QIcon::fromTheme("media-playback-start"));
    stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));

    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    ddAction->setCheckable(true);

    if (SETTINGS.value("Paths/ddiplrom", "").toString() != "") {
        if (SETTINGS.value("Emulation/64dd", "").toString() == "true")
            ddAction->setChecked(true);
    } else
        ddAction->setEnabled(false);

    menuBar->addMenu(emulationMenu);

    connect(startAction, SIGNAL(triggered()), this, SLOT(launchRomFromMenu()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(ddAction, SIGNAL(triggered()), this, SLOT(update64DD()));
    connect(logAction, SIGNAL(triggered()), this, SLOT(openLog()));


    //Settings
    settingsMenu = new QMenu(tr("&Settings"), this);
    configureAction = settingsMenu->addAction(tr("&Configure..."));
    configureAction->setIcon(QIcon::fromTheme("preferences-other"));

    menuBar->addMenu(settingsMenu);

    connect(configureAction, SIGNAL(triggered()), this, SLOT(openSettings()));


    //View
    viewMenu = new QMenu(tr("&View"), this);
    layoutMenu = viewMenu->addMenu(tr("&Layout"));
    layoutGroup = new QActionGroup(this);

    QList<QStringList> layouts;
    layouts << (QStringList() << tr("None")       << "none")
            << (QStringList() << tr("Table View") << "table")
            << (QStringList() << tr("Grid View")  << "grid")
            << (QStringList() << tr("List View")  << "list");

    QString layoutValue = SETTINGS.value("View/layout", "none").toString();

    foreach (QStringList layoutName, layouts)
    {
        QAction *layoutItem = layoutMenu->addAction(layoutName.at(0));
        layoutItem->setData(layoutName.at(1));
        layoutItem->setCheckable(true);
        layoutGroup->addAction(layoutItem);

        //Only enable layout changes when emulator is not running
        menuEnable << layoutItem;

        if(layoutValue == layoutName.at(1))
            layoutItem->setChecked(true);
    }

    viewMenu->addSeparator();

#if QT_VERSION >= 0x050000
    //OSX El Capitan adds it's own full-screen option
    if (QSysInfo::macVersion() < QSysInfo::MV_ELCAPITAN || QSysInfo::macVersion() == QSysInfo::MV_None)
        fullScreenAction = viewMenu->addAction(tr("&Full-screen"));
    else
        fullScreenAction = new QAction(this);
    statusBarAction = viewMenu->addAction(tr("&Status Bar"));
#else
    fullScreenAction = viewMenu->addAction(tr("&Full-screen"));
#endif

    fullScreenAction->setCheckable(true);
    statusBarAction->setCheckable(true);

    if (SETTINGS.value("View/fullscreen", "") == "true")
     fullScreenAction->setChecked(true);
    if (SETTINGS.value("View/statusbar", "") == "true")
        statusBarAction->setChecked(true);

    menuBar->addMenu(viewMenu);

    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(updateFullScreenMode()));
    connect(statusBarAction, SIGNAL(triggered()), this, SLOT(updateStatusBarView()));
    connect(layoutGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateLayoutSetting()));


    //Help
    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);

    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));


    //Create list of actions that are enabled only when emulator is not running
    menuEnable << startAction
               << ddAction
               << logAction
               << openAction
               << convertAction
               << downloadAction
               << deleteAction
               << refreshAction
               << configureAction
               << quitAction;

    //Create list of actions that are disabled when emulator is not running
    menuDisable << stopAction;

    //Create list of actions that are only active when a ROM is selected
    menuRomSelected << startAction
                    << deleteAction
                    << downloadAction;
}


void MainWindow::createRomView()
{
    //Create empty view
    emptyView = new QScrollArea(this);
    emptyView->setStyleSheet("QScrollArea { border: none; }");
    emptyView->setBackgroundRole(QPalette::Base);
    emptyView->setAutoFillBackground(true);
    emptyView->setHidden(true);

    emptyLayout = new QGridLayout(emptyView);

    emptyIcon = new QLabel(emptyView);
    emptyIcon->setPixmap(QPixmap(":/images/"+ParentNameLower+".png"));

    emptyLayout->addWidget(emptyIcon, 1, 1);
    emptyLayout->setColumnStretch(0, 1);
    emptyLayout->setColumnStretch(2, 1);
    emptyLayout->setRowStretch(0, 1);
    emptyLayout->setRowStretch(2, 1);

    emptyView->setLayout(emptyLayout);


    //Create table view
    tableView = new TableView(this);
    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromTable()));
    connect(tableView, SIGNAL(tableActive()), this, SLOT(enableButtons()));
    connect(tableView, SIGNAL(enterPressed()), this, SLOT(launchRomFromTable()));

    //Create grid view
    gridView = new GridView(this);
    connect(gridView, SIGNAL(gridItemSelected(bool)), this, SLOT(toggleMenus(bool)));


    //Create list view
    listView = new ListView(this);
    connect(listView, SIGNAL(listItemSelected(bool)), this, SLOT(toggleMenus(bool)));


    //Create disabled view
    disabledView = new QWidget(this);
    disabledView->setHidden(true);
    disabledView->setDisabled(true);

    disabledLayout = new QVBoxLayout(disabledView);
    disabledLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    disabledView->setLayout(disabledLayout);

    QString disabledText = QString("Add a directory containing ROMs under ")
                         + "Settings->Configure->Paths to use this view.";
    disabledLabel = new QLabel(disabledText, disabledView);
    disabledLabel->setWordWrap(true);
    disabledLabel->setAlignment(Qt::AlignCenter);
    disabledLayout->addWidget(disabledLabel);


    //Create 64DD view
    ddView = new DDView(this);
    connect(ddView, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(ddView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromMenu()));


    //Put views within a splitter for 64DD view
    viewSplitter = new QSplitter(this);
    viewSplitter->setOrientation(Qt::Vertical);
    viewSplitter->addWidget(emptyView);
    viewSplitter->addWidget(tableView);
    viewSplitter->addWidget(gridView);
    viewSplitter->addWidget(listView);
    viewSplitter->addWidget(disabledView);
    viewSplitter->addWidget(ddView);


    //Restore 64DD panel size
    QStringList sizes = SETTINGS.value("View/64ddsize", "").toString().split("|");
    foreach (QString size, sizes)
        sizeInts << size.toInt();

    restoreSplitterSize();


    showActiveView();
}


void MainWindow::disableButtons()
{
    toggleMenus(false);
}


void MainWindow::disableViews(bool imageUpdated)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    //Save position in current layout
    if (visibleLayout == "table")
        tableView->saveTablePosition();
    else if (visibleLayout == "grid")
        gridView->saveGridPosition();
    else if (visibleLayout == "list")
        listView->saveListPosition();

    resetLayouts(imageUpdated);
    tableView->clear();
    ddView->clear();

    if (ddAction->isChecked()) { //64DD enabled so show "No Cart" options
        Rom dummyRom;
        dummyRom.imageExists = false;
        tableView->addNoCartRow();
        gridView->addToGridView(&dummyRom, -1, ddAction->isChecked());
        listView->addToListView(&dummyRom, -1, ddAction->isChecked());
        ddView->addNoDiskRow();
    }

    tableView->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);
    ddView->setEnabled(false);

    foreach (QAction *next, menuRomSelected)
        next->setEnabled(false);
}


void MainWindow::enableButtons()
{
    toggleMenus(true);
}


void MainWindow::enableViews(int romCount, bool cached)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (romCount != 0) { //else no ROMs, so leave views disabled
        QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

        if (tableVisible.join("") != "")
            tableView->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);
        ddView->setEnabled(true);

        if (visibleLayout == "table")
            tableView->setFocus();
        else if (visibleLayout == "grid")
            gridView->setFocus();
        else if (visibleLayout == "list")
            listView->setFocus();

        //Check if disabled view is showing. If it is, re-enabled the selected view
        if (!disabledView->isHidden()) {
            disabledView->setHidden(true);
            showActiveView();
        }

        if (cached) {
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(0);
            timer->start();

            if (visibleLayout == "table")
                connect(timer, SIGNAL(timeout()), tableView, SLOT(setTablePosition()));
            else if (visibleLayout == "grid")
                connect(timer, SIGNAL(timeout()), gridView, SLOT(setGridPosition()));
            else if (visibleLayout == "list")
                connect(timer, SIGNAL(timeout()), listView, SLOT(setListPosition()));
        }
    } else {
        if (visibleLayout != "none") {
            tableView->setHidden(true);
            gridView->setHidden(true);
            listView->setHidden(true);
            disabledView->setHidden(false);
        }
    }
}


bool MainWindow::eventFilter(QObject*, QEvent *event)
{
    //Show menu bar if mouse is at top of screen in full-screen mode
    if (event->type() == QEvent::HoverMove && isFullScreen()) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        //x and y axis are reversed in Qt4
#if QT_VERSION >= 0x050000
        int mousePos = mouseEvent->pos().y();
#else
        int mousePos = mouseEvent->pos().x();
#endif

        if (mousePos < 5)
            showMenuBar(true);
        if (mousePos > 30)
            showMenuBar(false);
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        //Exit fullscreen mode if Esc key is pressed
        if (keyEvent->key() == Qt::Key_Escape && isFullScreen())
            updateFullScreenMode();
    }

#if QT_VERSION >= 0x050000
    //OSX El Capitan adds it's own full-screen option, so handle the event change here
    if (QSysInfo::macVersion() >= QSysInfo::MV_ELCAPITAN && QSysInfo::macVersion() != QSysInfo::MV_None) {
        if (event->type() == QEvent::WindowStateChange) {
            QWindowStateChangeEvent *windowEvent = static_cast<QWindowStateChangeEvent*>(event);

            if (windowEvent->oldState() == Qt::WindowNoState) {
                SETTINGS.setValue("View/fullscreen", true);
                tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            } else {
                SETTINGS.setValue("View/fullscreen", "");
                tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            }
        }
    }
#endif

    return false;
}


QString MainWindow::getCurrentRomInfoFromView(QString infoName)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        return tableView->getCurrentRomInfo(infoName);
    else if (visibleLayout == "grid" && gridView->hasSelectedRom())
        return gridView->getCurrentRomInfo(infoName);
    else if (visibleLayout == "list" && listView->hasSelectedRom())
        return listView->getCurrentRomInfo(infoName);

    return "";
}


void MainWindow::launchRom(QDir romDir, QString romFileName, QString zipFileName)
{
    if (ddAction->isChecked() && ddView->hasSelectedRom()) {
        QString ddFileName = ddView->getCurrentRomInfo("fileName");
        QString ddDirName = ddView->getCurrentRomInfo("dirName");
        QString ddZipName = ddView->getCurrentRomInfo("zipFile");

        emulation->startEmulator(romDir, romFileName, zipFileName, QDir(ddDirName), ddFileName, ddZipName);
    } else
        emulation->startEmulator(romDir, romFileName, zipFileName);
}


void MainWindow::launchRomFromMenu()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();

    if (visibleLayout == "table")
        launchRomFromTable();
    else if (visibleLayout == "grid")
        launchRomFromWidget(gridView->getCurrentRomWidget());
    else if (visibleLayout == "list")
        launchRomFromWidget(listView->getCurrentRomWidget());
}


void MainWindow::launchRomFromTable()
{
    if (tableView->hasSelectedRom()) {
        QString romFileName = tableView->getCurrentRomInfo("fileName");
        QString romDirName = tableView->getCurrentRomInfo("dirName");
        QString zipFileName = tableView->getCurrentRomInfo("zipFile");

        launchRom(QDir(romDirName), romFileName, zipFileName);
    } else {
        launchRom(QDir(), "", "");
    }
}


void MainWindow::launchRomFromWidget(QWidget *current)
{
    QString romFileName = current->property("fileName").toString();
    QString romDirName = current->property("directory").toString();
    QString zipFileName = current->property("zipFile").toString();
    launchRom(QDir(romDirName), romFileName, zipFileName);
}


void MainWindow::launchRomFromZip()
{
    QString fileName = zipList->currentItem()->text();
    zipDialog->close();

    emulation->startEmulator(QDir(), fileName, openPath);
}


void MainWindow::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void MainWindow::openConverter()
{
    V64Converter v64converter(romCollection->romPaths.at(0), this);
}


void MainWindow::openDeleteDialog()
{
    scraper = new TheGamesDBScraper(this);
    scraper->deleteGameInfo(getCurrentRomInfoFromView("fileName"), getCurrentRomInfoFromView("romMD5"));
    delete scraper;

    romCollection->cachedRoms();
}


void MainWindow::openDownloader()
{
    DownloadDialog downloadDialog(getCurrentRomInfoFromView("fileName"),
                                  getCurrentRomInfoFromView("search"),
                                  getCurrentRomInfoFromView("romMD5"),
                                  this);
    downloadDialog.exec();

    romCollection->cachedRoms();
}


void MainWindow::openLog()
{
    if (emulation->lastOutput == "") {
        QMessageBox::information(this, tr("No Output"),
            tr("There is no log. Either <ParentName> has not yet run or there was no output from the last run.")
            .replace("<ParentName>",ParentName));
    } else {
        LogDialog logDialog(emulation->lastOutput, this);
        logDialog.exec();
    }
}


void MainWindow::openSettings()
{
    QString tableImageBefore = SETTINGS.value("Table/imagesize", "Medium").toString();
    QString columnsBefore = SETTINGS.value("Table/columns", "Filename|Size").toString();
    QString downloadBefore = SETTINGS.value("Other/downloadinfo", "").toString();

    SettingsDialog settingsDialog(this, 0);
    settingsDialog.exec();

    QString tableImageAfter = SETTINGS.value("Table/imagesize", "Medium").toString();
    QString columnsAfter = SETTINGS.value("Table/columns", "Filename|Size").toString();
    QString downloadAfter = SETTINGS.value("Other/downloadinfo", "").toString();

    //Reset columns widths if user has selected different columns to display
    if (columnsBefore != columnsAfter) {
        SETTINGS.setValue("Table/width", "");
        tableView->setColumnCount(3);
        tableView->setHeaderLabels(QStringList(""));
    }

    QStringList romSave = SETTINGS.value("Paths/roms","").toString().split("|");
    if (romCollection->romPaths != romSave) {
        romCollection->updatePaths(romSave);
        romCollection->addRoms();
    } else if (downloadBefore == "" && downloadAfter == "true") {
        romCollection->addRoms();
    } else {
        if (tableImageBefore != tableImageAfter)
            romCollection->cachedRoms(true);
        else
            romCollection->cachedRoms(false);
    }

    if (SETTINGS.value("Paths/ddiplrom", "").toString() != "") {
        ddAction->setEnabled(true);
    } else {
        ddAction->setEnabled(false);
        ddAction->setChecked(false);
        update64DD();
    }

    gridView->setGridBackground();
    listView->setListBackground();
    toggleMenus(true);
}


void MainWindow::openRom()
{
    QString filter = "N64 ROMs (";
    foreach (QString type, romCollection->getFileTypes(true)) filter += type + " ";
    filter += ");;" + tr("All Files") + " (*)";

#if QT_VERSION >= 0x050000
    QString searchPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
#else
    QString searchPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif
    if (romCollection->romPaths.count() > 0)
        searchPath = romCollection->romPaths.at(0);

    openPath = QFileDialog::getOpenFileName(this, tr("Open ROM File"), searchPath, filter);
    if (openPath != "") {
        if (QFileInfo(openPath).suffix() == "zip") {
            QStringList zippedFiles = getZippedFiles(openPath);

            QString last;
            int count = 0;

            foreach (QString file, zippedFiles) {
                QString ext = file.right(4).toLower();

                if (romCollection->getFileTypes().contains("*" + ext)) {
                    last = file;
                    count++;
                }
            }

            if (count == 0)
                QMessageBox::information(this, tr("No ROMs"), tr("No ROMs found in ZIP file."));
            else if (count == 1)
                emulation->startEmulator(QDir(QFileInfo(openPath).dir()), last, openPath);
            else { //More than one ROM in zip file, so let user select
                openZipDialog(zippedFiles);
            }
        } else
            emulation->startEmulator(QDir(QFileInfo(openPath).dir()), openPath);
    }
}


void MainWindow::openZipDialog(QStringList zippedFiles)
{
    zipDialog = new QDialog(this);
    zipDialog->setWindowTitle(tr("Select ROM"));
    zipDialog->setMinimumSize(200, 150);
    zipDialog->resize(300, 150);

    zipLayout = new QGridLayout(zipDialog);
    zipLayout->setContentsMargins(5, 10, 5, 10);

    zipList = new QListWidget(zipDialog);
    foreach (QString file, zippedFiles) {
        QString ext = file.right(4);

        if (romCollection->getFileTypes().contains("*" + ext))
            zipList->addItem(file);
    }
    zipList->setCurrentRow(0);

    zipButtonBox = new QDialogButtonBox(Qt::Horizontal, zipDialog);
    zipButtonBox->addButton(tr("Launch"), QDialogButtonBox::AcceptRole);
    zipButtonBox->addButton(QDialogButtonBox::Cancel);

    zipLayout->addWidget(zipList, 0, 0);
    zipLayout->addWidget(zipButtonBox, 1, 0);

    connect(zipList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromZip()));
    connect(zipButtonBox, SIGNAL(accepted()), this, SLOT(launchRomFromZip()));
    connect(zipButtonBox, SIGNAL(rejected()), zipDialog, SLOT(close()));

    zipDialog->setLayout(zipLayout);

    zipDialog->exec();
}


void MainWindow::resetLayouts(bool imageUpdated)
{
    tableView->resetView(imageUpdated);
    gridView->resetView();
    listView->resetView();
}


void MainWindow::restoreSplitterSize()
{
    int mainSize = 1, ddSize = 1;

    if (!sizeInts.isEmpty()) {
        ddSize = sizeInts.last();
        sizeInts.removeLast();
    }

    foreach (int size, sizeInts)
    {
        if (size != 0) {
            mainSize = size; //Set all views to same height in case user switches view
            break;
        }
    }

    if (ddSize == 0) ddSize = 1;
    sizeInts.clear();
    sizeInts << mainSize << mainSize << mainSize << mainSize << mainSize << ddSize;
    viewSplitter->setSizes(sizeInts);
}


void MainWindow::showActiveView()
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        tableView->setHidden(false);
    else if (visibleLayout == "grid")
        gridView->setHidden(false);
    else if (visibleLayout == "list")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);
}


void MainWindow::showMenuBar(bool mouseAtTop)
{
    menuBar->setHidden(!mouseAtTop);
}


void MainWindow::showRomMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);

    QAction *contextStartAction = contextMenu->addAction(tr("&Start"));
    contextStartAction->setIcon(QIcon::fromTheme("media-playback-start"));

    connect(contextStartAction, SIGNAL(triggered()), this, SLOT(launchRomFromMenu()));

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        contextMenu->addSeparator();
        QAction *contextDownloadAction = contextMenu->addAction(tr("&Download/Update Info..."));
        QAction *contextDeleteAction = contextMenu->addAction(tr("D&elete Current Info..."));

        connect(contextDownloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
        connect(contextDeleteAction, SIGNAL(triggered()), this, SLOT(openDeleteDialog()));
    }


    QWidget *activeWidget = new QWidget(this);
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        activeWidget = tableView->viewport();
    else if (visibleLayout == "grid")
        activeWidget = gridView->getCurrentRomWidget();
    else if (visibleLayout == "list")
        activeWidget = listView->getCurrentRomWidget();

    contextMenu->exec(activeWidget->mapToGlobal(pos));
}


void MainWindow::stopEmulator()
{
    emulation->stopEmulator();
}


void MainWindow::toggleMenus(bool active)
{
    foreach (QAction *next, menuEnable)
        next->setEnabled(active);

    foreach (QAction *next, menuDisable)
        next->setEnabled(!active);

    tableView->setEnabled(active);
    gridView->setEnabled(active);
    listView->setEnabled(active);

    if (!tableView->hasSelectedRom() &&
        !gridView->hasSelectedRom() &&
        !listView->hasSelectedRom() &&
        !ddView->hasSelectedRom()
    ) {
        foreach (QAction *next, menuRomSelected)
            next->setEnabled(false);
    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "") {
        downloadAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }

    if (SETTINGS.value("Paths/ddiplrom", "").toString() == "")
        ddAction->setEnabled(false);
}


void MainWindow::updateFullScreenMode()
{
    if (isFullScreen()) {
        fullScreenAction->setChecked(false);
        SETTINGS.setValue("View/fullscreen", "");

        menuBar->setHidden(false);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        showNormal();
    } else {
        fullScreenAction->setChecked(true);
        SETTINGS.setValue("View/fullscreen", true);

        menuBar->setHidden(true);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        showFullScreen();
    }
}


void MainWindow::updateLayoutSetting()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();
    SETTINGS.setValue("View/layout", visibleLayout);

    sizeInts.clear();
    foreach(int size, viewSplitter->sizes())
        sizeInts << size;

    emptyView->setHidden(true);
    tableView->setHidden(true);
    gridView->setHidden(true);
    listView->setHidden(true);
    disabledView->setHidden(true);
    ddView->setHidden(true);

    int romCount = romCollection->cachedRoms();

    if (romCount > 0 || visibleLayout == "none")
        showActiveView();

    //Don't show 64DD panel for empty view
    QString ddipl = SETTINGS.value("Paths/ddiplrom", "").toString();
    QString ddEnabled = SETTINGS.value("Emulation/64dd", "").toString();

    if (visibleLayout != "none" && ddipl != "" && ddEnabled == "true") {
        ddView->setHidden(false);
        restoreSplitterSize();
    }

    //View was updated so no ROM will be selected. Update menu items accordingly
    foreach (QAction *next, menuRomSelected)
        next->setEnabled(false);
}


void MainWindow::update64DD()
{
    QString ddipl = SETTINGS.value("Paths/ddiplrom", "").toString();

    if(ddAction->isChecked() && ddipl != "") {
        SETTINGS.setValue("Emulation/64dd", true);

        if (SETTINGS.value("View/layout", "none").toString() != "none")
            ddView->setHidden(false);

        viewSplitter->setSizes(QList<int>() << 500 << 500 << 500 << 500 << 500 << 100);
    } else {
        SETTINGS.setValue("Emulation/64dd", "");
        ddView->setHidden(true);
    }

    romCollection->cachedRoms();
}


void MainWindow::updateStatusBar(QString message, int timeout)
{
    statusBar->showMessage(message, timeout);
}


void MainWindow::updateStatusBarView()
{
    if(statusBarAction->isChecked()) {
        SETTINGS.setValue("View/statusbar", true);
        statusBar->show();
    } else {
        SETTINGS.setValue("View/statusbar", "");
        statusBar->hide();
    }
}
