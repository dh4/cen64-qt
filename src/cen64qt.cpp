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

    if (SETTINGS.value("statusbar", "") == "")
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
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64",
                                                 QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
                {
                    QFile file(romDir.absoluteFilePath(*it));
                    qint64 size = QFileInfo(file).size();

                    fileItem = new QTreeWidgetItem;
                    fileItem->setText(0, QFileInfo(file).fileName());
                    fileItem->setText(1, tr("%1 MB").arg(int((size + 1023) / 1024 / 1024)));
                    fileItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

                    romTree->addTopLevelItem(fileItem);
                }
            } else {
            QMessageBox::warning(this, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Failed to open ROM directory.");
        }
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
    fileMenu->addSeparator();
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

    for(QStringList::Iterator it = inputs.begin(); it != inputs.end(); ++it)
    {
        QAction *input = inputMenu->addAction(*it);
        input->setData(QVariant(*it));
        input->setCheckable(true);
        inputGroup->addAction(input);

        //Only enable input actions when CEN64 is not running
        menuEnable << input;

        if(inputValue == *it)
            input->setChecked(true);
    }

    settingsMenu->addSeparator();
    optionsAction = settingsMenu->addAction(tr("&Paths"));

    menuBar->addMenu(settingsMenu);


    viewMenu = new QMenu(tr("&View"), this);
    statusBarAction = viewMenu->addAction(tr("&Status Bar"));
    outputAction = viewMenu->addAction(tr("&Output to Console"));

    statusBarAction->setCheckable(true);
    outputAction->setCheckable(true);

    if (SETTINGS.value("statusbar", "") == "true")
        statusBarAction->setChecked(true);

    if (SETTINGS.value("consoleoutput", "") == "true")
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
               << optionsAction
               << outputAction;

    //Create list of actions that are disabled when CEN64 is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(convertAction, SIGNAL(triggered()), this, SLOT(openConverter()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromRomTree()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(openOptions()));
    connect(statusBarAction, SIGNAL(triggered()), this, SLOT(updateStatusBarView()));
    connect(outputAction, SIGNAL(triggered()), this, SLOT(updateOutputView()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(inputGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateInputSetting()));
}


void CEN64Qt::createRomView()
{
    romTree = new QTreeWidget(this);
    romTree->setColumnCount(2);
    romTree->setWordWrap(false);
    romTree->setAllColumnsShowFocus(true);
    romTree->setRootIsDecorated(false);
    romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerItem = new QTreeWidgetItem;
    headerItem->setText(0, tr("ROM"));
    headerItem->setText(1, tr("Size"));
    headerItem->setTextAlignment(0, Qt::AlignCenter | Qt::AlignVCenter);
    headerItem->setTextAlignment(1, Qt::AlignCenter | Qt::AlignVCenter);
    romTree->setHeaderItem(headerItem);

    romTree->header()->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    romTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    romTree->header()->setResizeMode(0, QHeaderView::Stretch);
#endif
    romTree->setColumnWidth(1, 100);

    addRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
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


void CEN64Qt::openOptions() {
    PathsDialog pathsDialog(this);
    pathsDialog.exec();

    QString romSave = SETTINGS.value("Paths/roms","").toString();
    if (romPath != romSave) {
        romPath = romSave;
        romDir = QDir(romPath);
        addRoms();
    }
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

    if (SETTINGS.value("Paths/individualsave", "").toString() == "true") {
        QString eepromPath = SETTINGS.value("Paths/eeprom", "").toString();
        QString sramPath = SETTINGS.value("Paths/sram", "").toString();

        if (eepromPath != "")
            args << "-eeprom" << eepromPath;

        if (sramPath != "")
            args << "-sram" << sramPath;
    } else {
        QString savesPath = SETTINGS.value("Paths/saves", "").toString();
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


void CEN64Qt::stopEmulator()
{
    cen64proc->terminate();
}


void CEN64Qt::toggleMenus(bool active)
{
    QListIterator<QAction*> enableIter(menuEnable);
    while(enableIter.hasNext())
        enableIter.next()->setEnabled(active);

    QListIterator<QAction*> disableIter(menuDisable);
    while(disableIter.hasNext())
        disableIter.next()->setEnabled(!active);

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
        SETTINGS.setValue("consoleoutput", true);
    else
        SETTINGS.setValue("consoleoutput", "");
}


void CEN64Qt::updateStatusBarView() {
    if(statusBarAction->isChecked()) {
        SETTINGS.setValue("statusbar", true);
        statusBar->show();
    } else {
        SETTINGS.setValue("statusbar", "");
        statusBar->hide();
    }
}
