/***
 * Copyright (c) 2013, Presence
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

#include "cen64qt.h"
#include "aboutdialog.h"
#include "global.h"
#include "settingsdialog.h"


CEN64Qt::CEN64Qt(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("CEN64-Qt"));
    setWindowIcon(QIcon(":/images/cen64.png"));

    romPath = SETTINGS.value("Paths/roms","").toString();
    romDir = QDir(romPath);

    widget = new QWidget(this);
    setCentralWidget(widget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    createMenu();
    createRomView();

    statusBar = new QStatusBar;

    if (SETTINGS.value("View/statusbar", "") == "")
        statusBar->hide();

    layout = new QVBoxLayout(widget);
    layout->setMenuBar(menuBar);
    layout->addWidget(romTree);
    layout->addWidget(statusBar);
    layout->setMargin(0);

    widget->setLayout(layout);
    widget->setMinimumSize(300, 200);
}


void CEN64Qt::addRoms()
{
    SETTINGS.setValue("ROMs/cache", "");
    QStringList visible = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");
    resetRomTreeLayout(visible);

    romTree->setEnabled(false);
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64",
                                                 QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                QProgressDialog progress("Loading ROMs...", "Cancel", 0, files.size(), this);
#if QT_VERSION >= 0x050000
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowMinimizeButtonHint);
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
#else
                progress.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
#endif
                progress.setCancelButton(0);
                progress.show();
                progress.setWindowModality(Qt::WindowModal);

                int count = 0;
                foreach (QString fileName, files)
                {
                    QFile file(romDir.absoluteFilePath(fileName));

                    file.open(QIODevice::ReadOnly);
                    romData = new QByteArray(file.readAll());
                    file.close();

                    *romData = byteswap(*romData);

                    QString internalName = QString(romData->mid(32, 20)).trimmed();

                    QString romMD5 = QString(QCryptographicHash::hash(*romData,
                                             QCryptographicHash::Md5).toHex());

                    delete romData;

                    if (visible.join("") != "") //Otherwise no columns, so don't bother populating
                        addToRomTree(fileName, romMD5, internalName, visible);

                    QString currentSetting = SETTINGS.value("ROMs/cache", "").toString();
                    QString newSetting = currentSetting
                                         + fileName + "|"
                                         + internalName + "|"
                                         + romMD5 + "||";

                    SETTINGS.setValue("ROMs/cache", newSetting);

                    count++;
                    progress.setValue(count);
                }
            } else {
            QMessageBox::warning(this, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Failed to open ROM directory.");
        }
    }

    if (visible.join("") != "")
        romTree->setEnabled(true);
}


void CEN64Qt::addToRomTree(QString fileName, QString romMD5, QString internalName, QStringList visible)
{
    QFile file(romDir.absoluteFilePath(fileName));
    qint64 size = QFileInfo(file).size();

    QString catalogFile = SETTINGS.value("Paths/catalog", "").toString();

    bool getGoodName = false;
    if (QFileInfo(catalogFile).exists()) {
        romCatalog = new QSettings(catalogFile, QSettings::IniFormat);
        getGoodName = true;
    }

    QString goodName = "Requires catalog file";
    QString CRC1 = "";
    QString CRC2 = "";
    QString players = "";
    QString saveType = "";
    QString rumble = "";

    romMD5 = romMD5.toUpper();

    if (getGoodName) {
        //Join GoodName on ", ", otherwise entries with a comma won't show
        QVariant goodNameVariant = romCatalog->value(romMD5+"/GoodName","Unknown ROM");
        goodName = goodNameVariant.toStringList().join(", ");

        QStringList CRC = romCatalog->value(romMD5+"/CRC","").toString().split(" ");

        if (CRC.size() == 2) {
            CRC1 = CRC[0];
            CRC2 = CRC[1];
        }

        QString newMD5 = romCatalog->value(romMD5+"/RefMD5","").toString();
        if (newMD5 == "")
            newMD5 = romMD5;

        players = romCatalog->value(newMD5+"/Players","").toString();
        saveType = romCatalog->value(newMD5+"/SaveType","").toString();
        rumble = romCatalog->value(newMD5+"/Rumble","").toString();
    }

    fileItem = new TreeWidgetItem(romTree);
    fileItem->setText(0, fileName);

    int i = 1;
    foreach (QString current, visible)
    {
        if (current == "GoodName") {
            fileItem->setText(i, goodName);
            if (goodName == "Unknown ROM" || goodName == "Requires catalog file")
                fileItem->setForeground(i, QBrush(Qt::gray));
        }
        else if (current == "Filename") {
            fileItem->setText(i, QFileInfo(file).completeBaseName());
        }
        else if (current == "Filename (extension)") {
            fileItem->setText(i, fileName);
        }
        else if (current == "Internal Name") {
            fileItem->setText(i, internalName);
        }
        else if (current == "Size") {
            fileItem->setText(i, tr("%1 MB").arg(int((size + 1023) / 1024 / 1024)));
            fileItem->setData(i, Qt::UserRole, (int)size);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "MD5") {
            fileItem->setText(i, romMD5.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "CRC1") {
            fileItem->setText(i, CRC1.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "CRC2") {
            fileItem->setText(i, CRC2.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Players") {
            fileItem->setText(i, players);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Rumble") {
            fileItem->setText(i, rumble);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Save Type") {
            fileItem->setText(i, saveType);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else //Invalid column name in config file
            fileItem->setText(i, "");

        i++;
    }

    romTree->addTopLevelItem(fileItem);
}


QByteArray CEN64Qt::byteswap(QByteArray romData)
{
        QByteArray flipped;

        if (romData.left(4).toHex() == "37804012") {
            for (int i = 0; i < romData.length(); i += 2)
            {
                flipped.append(romData[i + 1]);
                flipped.append(romData[i]);
            }
            return flipped;
        } else {
            return romData;
        }
}


void CEN64Qt::cachedRoms()
{
    romTree->setEnabled(false);
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    QStringList visible = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");
    resetRomTreeLayout(visible);


    if (visible.join("") != "") { //Otherwise no columns, so don't bother populating

        QString cache = SETTINGS.value("ROMs/cache", "").toString();
        QStringList cachedRoms = cache.split("||");

        foreach (QString current, cachedRoms)
        {
            QStringList romInfo = current.split("|");

            if (romInfo.size() == 3) {
                addToRomTree(romInfo[0], romInfo[2], romInfo[1], visible);
            }
        }

        romTree->setEnabled(true);
    }
}


void CEN64Qt::checkStatus(int status)
{
    if (status > 0)
        QMessageBox::warning(this, "Warning",
            "CEN64 quit unexpectedly. Check to make sure you are using a valid ROM.");
    else
        statusBar->showMessage("Emulation stopped", 3000);
}


void CEN64Qt::closeEvent(QCloseEvent *event)
{
    SETTINGS.setValue("Geometry/windowx", x());
    SETTINGS.setValue("Geometry/windowy", y());
    SETTINGS.setValue("Geometry/width", width());
    SETTINGS.setValue("Geometry/height", height());
    if (isMaximized())
        SETTINGS.setValue("Geometry/maximized", true);
    else
        SETTINGS.setValue("Geometry/maximized", "");

    saveColumnWidths();

    event->accept();
}


void CEN64Qt::createMenu()
{
    menuBar = new QMenuBar(this);


    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open ROM..."));
    fileMenu->addSeparator();
    convertAction = fileMenu->addAction(tr("&Convert V64..."));
    refreshAction = fileMenu->addAction(tr("&Refresh List"));
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    fileMenu->addSeparator();
#endif
    quitAction = fileMenu->addAction(tr("&Quit"));

    openAction->setIcon(QIcon::fromTheme("document-open"));
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));

    menuBar->addMenu(fileMenu);


    emulationMenu = new QMenu(tr("&Emulation"), this);
    startAction = emulationMenu->addAction(tr("&Start"));
    stopAction = emulationMenu->addAction(tr("St&op"));

    startAction->setIcon(QIcon::fromTheme("media-playback-start"));
    stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));

    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    menuBar->addMenu(emulationMenu);


    settingsMenu = new QMenu(tr("&Settings"), this);
    inputMenu = settingsMenu->addMenu(tr("&Input"));
    inputMenu->setIcon(QIcon::fromTheme("input-gaming"));
    inputGroup = new QActionGroup(this);

    QStringList inputs;
    inputs << "keyboard" << "mayflash64" << "retrolink" << "wiiu" << "x360";

    QString inputValue = SETTINGS.value("input","keyboard").toString();

    foreach (QString inputName, inputs)
    {
        QAction *input = inputMenu->addAction(inputName);
        input->setData(inputName);
        input->setCheckable(true);
        inputGroup->addAction(input);

        //Only enable input actions when CEN64 is not running
        menuEnable << input;

        if(inputValue == inputName)
            input->setChecked(true);
    }

    settingsMenu->addSeparator();
    pathsAction = settingsMenu->addAction(tr("&Paths"));
    columnsAction = settingsMenu->addAction(tr("&Columns"));

    pathsAction->setIcon(QIcon::fromTheme("preferences-other"));
    columnsAction->setIcon(QIcon::fromTheme("tab-new"));

    menuBar->addMenu(settingsMenu);


    viewMenu = new QMenu(tr("&View"), this);
    statusBarAction = viewMenu->addAction(tr("&Status Bar"));
    outputAction = viewMenu->addAction(tr("&Output to Console"));

    statusBarAction->setCheckable(true);
    outputAction->setCheckable(true);

    if (SETTINGS.value("View/statusbar", "") == "true")
        statusBarAction->setChecked(true);

    if (SETTINGS.value("View/consoleoutput", "") == "true")
        outputAction->setChecked(true);

    menuBar->addMenu(viewMenu);


    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);

    //Create list of actions that are enabled only when CEN64 is not running
    menuEnable << startAction
               << openAction
               << convertAction
               << refreshAction
               << pathsAction
               << columnsAction
               << outputAction
               << quitAction;

    //Create list of actions that are disabled when CEN64 is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(convertAction, SIGNAL(triggered()), this, SLOT(openConverter()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromRomTree()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(pathsAction, SIGNAL(triggered()), this, SLOT(openPaths()));
    connect(columnsAction, SIGNAL(triggered()), this, SLOT(openColumns()));
    connect(statusBarAction, SIGNAL(triggered()), this, SLOT(updateStatusBarView()));
    connect(outputAction, SIGNAL(triggered()), this, SLOT(updateOutputView()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(inputGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateInputSetting()));
}


void CEN64Qt::createRomView()
{
    romTree = new QTreeWidget(this);
    romTree->setWordWrap(false);
    romTree->setAllColumnsShowFocus(true);
    romTree->setRootIsDecorated(false);
    romTree->setSortingEnabled(true);
    romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerView = new QHeaderView(Qt::Horizontal, this);
    romTree->setHeader(headerView);

    cachedRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void CEN64Qt::enableButtons()
{
    toggleMenus(true);
}


void CEN64Qt::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void CEN64Qt::openColumns()
{
    openOptions(1);
}


void CEN64Qt::openConverter() {
    QString v64File = QFileDialog::getOpenFileName(this, tr("Open v64 File"), romPath,
                                                   tr("V64 ROMs (*.v64 *.n64);;All Files (*)"));

    if (v64File != "") {
        QString defaultFileName = QFileInfo(QFile(v64File)).completeBaseName() + ".z64";
        QString defaultFile = romDir.absoluteFilePath(defaultFileName);
        QString saveFile = QFileDialog::getSaveFileName(this, tr("Save z64 File"), defaultFile,
                                                        tr("Z64 ROMs (*.z64);;All Files (*)"));

        if (saveFile != "")
            runConverter(v64File, saveFile);
    }
}


void CEN64Qt::openOptions(int activeTab) {

    QString columnsBefore = SETTINGS.value("ROMs/columns", "Filename|Size").toString();

    SettingsDialog settingsDialog(this, activeTab);
    settingsDialog.exec();

    QString columnsAfter = SETTINGS.value("ROMs/columns", "Filename|Size").toString();

    //Reset columns widths if user has selected different columns to display
    if (columnsBefore != columnsAfter) {
        SETTINGS.setValue("ROMs/width", "");
        romTree->setColumnCount(1);
        romTree->setHeaderLabels(QStringList(""));
    }

    QString romSave = SETTINGS.value("Paths/roms","").toString();
    if (romPath != romSave) {
        romPath = romSave;
        romDir = QDir(romPath);
        addRoms();
    } else {
        cachedRoms();
    }
}


void CEN64Qt::openPaths()
{
    openOptions(0);
}


void CEN64Qt::openRom()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath,
                                                tr("N64 ROMs (*.z64 *.n64);;All Files (*)"));
    if (path != "")
        runEmulator(path);
}


void CEN64Qt::readCEN64Output() {
    QString output = cen64proc->readAllStandardOutput();
    QStringList outputList = output.split("\n");

    int lastIndex = outputList.lastIndexOf(QRegExp("^.*VI/s.*MHz$"));

    if (lastIndex >= 0)
        statusBar->showMessage(outputList[lastIndex]);
}


void CEN64Qt::resetRomTreeLayout(QStringList visible)
{
    saveColumnWidths();
    QStringList widths = SETTINGS.value("ROMs/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << visible;
    romTree->setColumnCount(headerLabels.size());
    romTree->setHeaderLabels(headerLabels);
    headerView->setSortIndicatorShown(false);

    QStringList sort = SETTINGS.value("ROMs/sort", "").toString().split("|");
    if (sort.size() == 2) {
        if (sort[1] == "descending")
            headerView->setSortIndicator(visible.indexOf(sort[0]) + 1, Qt::DescendingOrder);
        else
            headerView->setSortIndicator(visible.indexOf(sort[0]) + 1, Qt::AscendingOrder);
    }

    romTree->setColumnHidden(0, true); //Hidden filename for launching emulator


    int i = 1;
    foreach (QString current, visible)
    {
        if (i == 1) {
            if (SETTINGS.value("ROMs/stretchfirstcolumn", "true") == "true") {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(i, QHeaderView::Stretch);
#else
                romTree->header()->setResizeMode(i, QHeaderView::Stretch);
#endif
            } else {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(i, QHeaderView::Interactive);
#else
                romTree->header()->setResizeMode(i, QHeaderView::Interactive);
#endif
            }
        }

        if (widths.size() == visible.size()) {
            romTree->setColumnWidth(i, widths[i - 1].toInt());
        } else {
            if (current == "GoodName" || current.left(8) == "Filename")
                romTree->setColumnWidth(i, 300);
            else if (current == "MD5")
                romTree->setColumnWidth(i, 250);
            else if (current == "Internal Name")
                romTree->setColumnWidth(i, 200);
            else if (current == "Save Type")
                romTree->setColumnWidth(i, 100);
            else if (current == "CRC1" || current == "CRC2")
                romTree->setColumnWidth(i, 90);
            else if (current == "Size" || current == "Rumble" || current == "Players")
                romTree->setColumnWidth(i, 75);
        }

        i++;
    }
}


void CEN64Qt::runConverter(QString v64File, QString saveFile) {
    QFile v64(v64File);
    v64.open(QIODevice::ReadOnly);

    QString v64Check(v64.read(4).toHex()), message;
    if (v64Check != "37804012") {
        if (v64Check == "80371240")
            message = "\"" + QFileInfo(v64).fileName() + "\" already in z64 format!";
        else
            message = "\"" + QFileInfo(v64).fileName() + "\" is not a valid .v64 file!";

        QMessageBox::warning(this, tr("CEN64-Qt Converter"), message);
    } else {
        v64.seek(0);

        QFile z64(saveFile);
        z64.open(QIODevice::WriteOnly);

        QByteArray data;
        QByteArray flipped;

        while (!v64.atEnd())
        {
            data = v64.read(1024);

            for (int i = 0; i < data.size(); i+=2)
            {
                //Check to see if only one byte remaining (though byte count should always be even)
                if (i + 1 == data.size())
                    flipped.append(data[i]);
                else {
                    flipped.append(data[i + 1]);
                    flipped.append(data[i]);
                }
            }

            z64.write(flipped);

            flipped.truncate(0);
        }

        z64.close();
        QMessageBox::information(this, tr("CEN64-Qt Converter"), tr("Conversion complete!"));
    }

    v64.close();
}


void CEN64Qt::runEmulator(QString completeRomPath)
{
    QString cen64Path = SETTINGS.value("Paths/cen64", "").toString();
    QString pifPath = SETTINGS.value("Paths/pifrom", "").toString();
    QString input = inputGroup->checkedAction()->data().toString();

    QFile cen64File(cen64Path);
    QFile pifFile(pifPath);
    QFile romFile(completeRomPath);

    if(QFileInfo(cen64File).exists() == false) {
        QMessageBox::warning(this, "Warning", "CEN64 executable not found.");
        return;
    }

    if(QFileInfo(pifFile).exists() == false) {
        QMessageBox::warning(this, "Warning", "PIFdata file not found.");
        return;
    }

    if(QFileInfo(romFile).exists() == false) {
        QMessageBox::warning(this, "Warning", "ROM file not found.");
        return;
    }

    QStringList args;
    args << "-controller" << input;

    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
        QString eepromPath = SETTINGS.value("Saves/eeprom", "").toString();
        QString sramPath = SETTINGS.value("Saves/sram", "").toString();

        if (eepromPath != "")
            args << "-eeprom" << eepromPath;

        if (sramPath != "")
            args << "-sram" << sramPath;
    } else {
        QString savesPath = SETTINGS.value("Saves/directory", "").toString();
        if (savesPath != "") {
            savesDir = QDir(savesPath);

            if (savesDir.exists()) {
                romFile.open(QIODevice::ReadOnly);
                romData = new QByteArray(romFile.readAll());
                romFile.close();

                QString romMD5 = QString(QCryptographicHash::hash(*romData,
                                                                  QCryptographicHash::Md5).toHex());

                QString romBaseName = QFileInfo(romFile).completeBaseName();
                QString eepromFileName = romBaseName + "." + romMD5 + ".eeprom";
                QString sramFileName = romBaseName + "." + romMD5 + ".sram";
                QString eepromPath = savesDir.absoluteFilePath(eepromFileName);
                QString sramPath = savesDir.absoluteFilePath(sramFileName);

                args << "-eeprom" << eepromPath << "-sram" << sramPath;

                delete romData;
            }
        }
    }

    args << pifPath << completeRomPath;

    toggleMenus(false);

    cen64proc = new QProcess(this);
    connect(cen64proc, SIGNAL(finished(int)), this, SLOT(enableButtons()));
    connect(cen64proc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    if (outputAction->isChecked())
        cen64proc->setProcessChannelMode(QProcess::ForwardedChannels);
    else
        connect(cen64proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readCEN64Output()));

    cen64proc->start(cen64Path, args);

    statusBar->showMessage("Emulation started", 3000);
}


void CEN64Qt::runEmulatorFromRomTree()
{
    QString completeRomFileName = QVariant(romTree->currentItem()->data(0, 0)).toString();
    QString completeRomPath = romDir.absoluteFilePath(completeRomFileName);
    runEmulator(completeRomPath);
}


void CEN64Qt::saveColumnWidths()
{
    QStringList widths;

    for (int i = 1; i < romTree->columnCount(); i++)
    {
        widths << QString::number(romTree->columnWidth(i));
    }

    if (widths.size() > 0)
        SETTINGS.setValue("ROMs/width", widths.join("|"));
}


void CEN64Qt::saveSortOrder(int column, Qt::SortOrder order)
{
    QString columnName = headerLabels.value(column);

    if (order == Qt::DescendingOrder)
        SETTINGS.setValue("ROMs/sort", columnName + "|descending");
    else
        SETTINGS.setValue("ROMs/sort", columnName + "|ascending");
}


void CEN64Qt::stopEmulator()
{
    cen64proc->terminate();
}


void CEN64Qt::toggleMenus(bool active)
{
    foreach (QAction *next, menuEnable)
        next->setEnabled(active);

    foreach (QAction *next, menuDisable)
        next->setEnabled(!active);

    romTree->setEnabled(active);

    if (romTree->currentItem() == NULL)
        startAction->setEnabled(false);
}


void CEN64Qt::updateInputSetting()
{
    SETTINGS.setValue("input", inputGroup->checkedAction()->data().toString());
}


void CEN64Qt::updateOutputView() {
    if(outputAction->isChecked())
        SETTINGS.setValue("View/consoleoutput", true);
    else
        SETTINGS.setValue("View/consoleoutput", "");
}


void CEN64Qt::updateStatusBarView() {
    if(statusBarAction->isChecked()) {
        SETTINGS.setValue("View/statusbar", true);
        statusBar->show();
    } else {
        SETTINGS.setValue("View/statusbar", "");
        statusBar->hide();
    }
}
