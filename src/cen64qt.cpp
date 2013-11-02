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

    romPath = SETTINGS.value("roms","").toString();
    romDir = QDir(romPath);

    widget = new QWidget(this);
    setCentralWidget(widget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    createMenu();
    createRomView();

    layout = new QVBoxLayout(widget);
    layout->setMenuBar(menuBar);
    layout->addWidget(romTree);
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
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64", QDir::Files | QDir::NoSymLinks);

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
    if (status > 0) {
        QString message = QString(cen64proc->readAllStandardOutput()); //CEN64 prints error message to stdout instead of stderr on Linux
        if (message == "") {
            message = "CEN64 quit unexpectedly. Check to make sure you are using a valid ROM.";
        }
        QMessageBox::warning(this, "Warning", message);
    }
}


void CEN64Qt::closeEvent(QCloseEvent *event)
{
    SETTINGS.setValue("Geometry/windowx", x());
    SETTINGS.setValue("Geometry/windowy", y());
    SETTINGS.setValue("Geometry/width", width());
    SETTINGS.setValue("Geometry/height", height());
    if (isMaximized()) {
        SETTINGS.setValue("Geometry/maximized", true);
    } else {
        SETTINGS.setValue("Geometry/maximized", "");
    }
    event->accept();
}


void CEN64Qt::createMenu()
{
    menuBar = new QMenuBar(this);


    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open ROM..."));
    fileMenu->addSeparator();
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
    inputs << "keyboard" << "mayflash64" << "retrolink" << "x360";

    QString inputValue = SETTINGS.value("input","keyboard").toString();

    for(QStringList::Iterator it = inputs.begin(); it != inputs.end(); ++it)
    {
        QAction *input = inputMenu->addAction(*it);
        input->setData(QVariant(*it));
        input->setCheckable(true);
        inputGroup->addAction(input);

        if(inputValue == *it) {
            input->setChecked(true);
        }
    }

    settingsMenu->addSeparator();
    optionsAction = settingsMenu->addAction(tr("&Paths"));

    menuBar->addMenu(settingsMenu);


    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);


    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromRomTree()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(openOptions()));
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
    startAction->setEnabled(true);
    stopAction->setEnabled(false);
    romTree->setEnabled(true);
}


void CEN64Qt::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void CEN64Qt::openOptions() {
    PathsDialog pathsDialog(this);
    pathsDialog.exec();

    if (romPath != SETTINGS.value("roms","").toString()) {
        romPath = SETTINGS.value("roms","").toString();
        romDir = QDir(romPath);
        addRoms();
    }
}


void CEN64Qt::openRom()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath, tr("N64 ROMs (*.z64 *.n64);;All Files (*)"));
    if (path != "")
        runEmulator(path);
}


void CEN64Qt::runEmulator(QString completeRomPath)
{
    QString cen64Path = SETTINGS.value("cen64", "").toString();
    QString pifPath = SETTINGS.value("pifrom", "").toString();
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

    if (SETTINGS.value("individualsave", "").toString() == "true") {
        QString eepromPath = SETTINGS.value("eeprom", "").toString();
        QString sramPath = SETTINGS.value("sram", "").toString();

        if (eepromPath != "") {
            args << "-eeprom" << eepromPath;
        }

        if (sramPath != "") {
            args << "-sram" << sramPath;
        }
    } else {
        QString savesPath = SETTINGS.value("saves", "").toString();
        if (savesPath != "") {
            savesDir = QDir(savesPath);

            if (savesDir.exists()) {
                romFile.open(QIODevice::ReadOnly);
                romData = new QByteArray(romFile.readAll());
                romFile.close();

                QString romMD5 = QString(QCryptographicHash::hash(*romData, QCryptographicHash::Md5).toHex());
                QString eepromPath = savesDir.absoluteFilePath(QString(QFileInfo(romFile).baseName() + "." + romMD5 + ".eeprom"));
                QString sramPath = savesDir.absoluteFilePath(QString(QFileInfo(romFile).baseName() + "." + romMD5 + ".sram"));

                args << "-eeprom" << eepromPath << "-sram" << sramPath;

                delete romData;
            }
        }
    }

    args << pifPath << completeRomPath;

    startAction->setEnabled(false);
    stopAction->setEnabled(true);
    romTree->setEnabled(false);

    cen64proc = new QProcess(this);
    connect(cen64proc, SIGNAL(finished(int)), this, SLOT(enableButtons()));
    connect(cen64proc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));
    cen64proc->start(cen64Path, args);
}


void CEN64Qt::runEmulatorFromRomTree()
{
    QString completeRomPath = romDir.absoluteFilePath(QVariant(romTree->currentItem()->data(0, 0)).toString());
    runEmulator(completeRomPath);
}


void CEN64Qt::stopEmulator()
{
    cen64proc->terminate();
}


void CEN64Qt::updateInputSetting()
{
    SETTINGS.setValue("input", inputGroup->checkedAction()->data().toString());
}
