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

#include "settingsdialog.h"
#include "global.h"


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(550);
    setMinimumHeight(400);

    layout = new QGridLayout(this);
    tabWidget = new QTabWidget(this);


    //Paths tab
    pathsWidget = new QWidget(this);
    pathsMainLayout = new QVBoxLayout(pathsWidget);

    paths = new QGroupBox(tr("CEN64 Files"), this);
    pathsLayout = new QGridLayout(paths);

    cen64PathLabel = new QLabel(tr("Path to CEN64 executable:"), this);
    pifPathLabel = new QLabel(tr("Path to pifrom/pifdata file:"), this);
    romPathLabel = new QLabel(tr("Path to ROM directory:"), this);
    catalogPathLabel = new QLabel(tr("Path to ROM catalog:"), this);

    cen64Path = new QLineEdit(SETTINGS.value("Paths/cen64", "").toString(), this);
    pifPath = new QLineEdit(SETTINGS.value("Paths/pifrom", "").toString(), this);
    romPath = new QLineEdit(SETTINGS.value("Paths/roms", "").toString(), this);
    catalogPath = new QLineEdit(SETTINGS.value("Paths/catalog", "").toString(), this);

    cen64Button = new QPushButton(tr("Browse..."), this);
    pifButton = new QPushButton(tr("Browse..."), this);
    romButton = new QPushButton(tr("Browse..."), this);
    catalogButton = new QPushButton(tr("Browse..."), this);

    pathsLayout->addWidget(cen64PathLabel, 0, 0);
    pathsLayout->addWidget(pifPathLabel, 1, 0);
    pathsLayout->addWidget(romPathLabel, 2, 0);
    pathsLayout->addWidget(catalogPathLabel, 3, 0);

    pathsLayout->addWidget(cen64Path, 0, 1);
    pathsLayout->addWidget(pifPath, 1, 1);
    pathsLayout->addWidget(romPath, 2, 1);
    pathsLayout->addWidget(catalogPath, 3, 1);

    pathsLayout->addWidget(cen64Button, 0, 2);
    pathsLayout->addWidget(pifButton, 1, 2);
    pathsLayout->addWidget(romButton, 2, 2);
    pathsLayout->addWidget(catalogButton, 3, 2);

    paths->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    paths->setLayout(pathsLayout);

    saves = new QGroupBox(tr("Save Files"), this);
    savesLayout = new QGridLayout(saves);

    savesPathLabel = new QLabel(tr("Save Directory:"), this);
    eepromPathLabel = new QLabel(tr("Path to EEPROM:"), this);
    sramPathLabel = new QLabel(tr("Path to SRAM:"), this);

    savesPath = new QLineEdit(SETTINGS.value("Saves/directory", "").toString(), this);
    eepromPath = new QLineEdit(SETTINGS.value("Saves/eeprom", "").toString(), this);
    sramPath = new QLineEdit(SETTINGS.value("Saves/sram", "").toString(), this);

    savesButton = new QPushButton(tr("Browse..."), this);
    eepromButton = new QPushButton(tr("Browse..."), this);
    sramButton = new QPushButton(tr("Browse..."), this);

    savesLayout->addWidget(savesPathLabel, 0, 0);
    savesLayout->addWidget(eepromPathLabel, 2, 0);
    savesLayout->addWidget(sramPathLabel, 3, 0);

    savesLayout->addWidget(savesPath, 0, 1);
    savesLayout->addWidget(eepromPath, 2, 1);
    savesLayout->addWidget(sramPath, 3, 1);

    savesLayout->addWidget(savesButton, 0, 2);
    savesLayout->addWidget(eepromButton, 2, 2);
    savesLayout->addWidget(sramButton, 3, 2);

    saveOption = new QCheckBox(tr("Specify &individual files for saves"), this);
    savesLayout->addWidget(saveOption, 1, 1);

    //Widgets enabled when checkbox is active
    saveEnable << eepromPathLabel
               << eepromPath
               << eepromButton
               << sramPathLabel
               << sramPath
               << sramButton;

    //Widgets disabled when checkbox is active
    saveDisable << savesPathLabel
                << savesPath
                << savesButton;

    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
        toggleSaves(true);
        saveOption->setChecked(true);
    } else
        toggleSaves(false);

    saves->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saves->setLayout(savesLayout);

    pathsMainLayout->addWidget(paths);
    pathsMainLayout->addWidget(saves);
    pathsWidget->setLayout(pathsMainLayout);


    //Columns tab
    columnsWidget = new QWidget(this);
    columnsLayout = new QGridLayout(columnsWidget);

    availableLabel = new QLabel(tr("Available:"), this);
    currentLabel = new QLabel(tr("Current:"), this);

    availableList = new QListWidget(this);
    currentList = new QListWidget(this);

    addButton = new QToolButton(this);
    removeButton = new QToolButton(this);
    sortUpButton = new QToolButton(this);
    sortDownButton = new QToolButton(this);

    addButton->setArrowType(Qt::RightArrow);
    removeButton->setArrowType(Qt::LeftArrow);
    sortUpButton->setArrowType(Qt::DownArrow);
    sortDownButton->setArrowType(Qt::UpArrow);

    available << "Filename"
              << "Filename (extension)"
              << "GoodName"
              << "Internal Name"
              << "Size"
              << "MD5"
              << "CRC1"
              << "CRC2"
              << "Players"
              << "Rumble"
              << "Save Type";

    current = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");

    foreach (QString cur, current)
    {
        if (available.contains(cur))
            available.removeOne(cur);
        else //Someone added an invalid item
            current.removeOne(cur);
    }

    availableList->setMaximumHeight(250);
    availableList->setMaximumWidth(160);
    availableList->addItems(available);
    availableList->sortItems();

    currentList->setMaximumHeight(250);
    currentList->setMaximumWidth(160);
    currentList->addItems(current);

    toggleWidget = new QWidget(this);
    toggleLayout = new QVBoxLayout(toggleWidget);
    toggleLayout->addWidget(addButton);
    toggleLayout->addWidget(removeButton);
    toggleWidget->setLayout(toggleLayout);
    toggleWidget->setMaximumHeight(80);

    sortWidget = new QWidget(this);
    sortLayout = new QVBoxLayout(sortWidget);
    sortLayout->addWidget(sortDownButton);
    sortLayout->addWidget(sortUpButton);
    sortWidget->setLayout(sortLayout);
    sortWidget->setMaximumHeight(80);

    stretchOption = new QCheckBox(tr("Stretch first column"), this);
    if (SETTINGS.value("ROMs/stretchfirstcolumn", "true").toString() == "true")
        stretchOption->setChecked(true);

    columnsLayout->addWidget(availableLabel, 0, 1);
    columnsLayout->addWidget(currentLabel, 0, 3);
    columnsLayout->addWidget(availableList, 1, 1);
    columnsLayout->addWidget(currentList, 1, 3);
    columnsLayout->addWidget(toggleWidget, 1, 2);
    columnsLayout->addWidget(sortWidget, 1, 4);
    columnsLayout->addWidget(stretchOption, 2, 3);

    columnsLayout->setColumnStretch(0, 1);
    columnsLayout->setColumnStretch(5, 1);
    columnsLayout->setRowStretch(3, 1);
    columnsWidget->setLayout(columnsLayout);


    //Other tab
#ifndef Q_OS_WIN
    otherWidget = new QWidget(this);
    otherLayout = new QGridLayout(otherWidget);

    outputLabel = new QLabel(tr("Output to Console:"), this);
    outputOption = new QCheckBox(this);

    QString outputToolTip = "If checked, CEN64's output will be redirected to CEN64-Qt ";
    outputToolTip        += "so you can view the VI/s or debug information.";
    outputOption->setToolTip(outputToolTip);

    if (SETTINGS.value("Other/consoleoutput", "").toString() == "true")
        outputOption->setChecked(true);

    otherLayout->addWidget(outputLabel, 1, 1);
    otherLayout->addWidget(outputOption, 1, 2);

    otherLayout->setColumnMinimumWidth(1, 120);
    otherLayout->setColumnMinimumWidth(0, 10);
    otherLayout->setRowMinimumHeight(0, 10);
    otherLayout->setColumnStretch(2, 1);
    otherLayout->setRowStretch(2, 1);
    otherWidget->setLayout(otherLayout);
#endif


    tabWidget->addTab(pathsWidget, "Paths");
    tabWidget->addTab(columnsWidget, "Columns");
#ifndef Q_OS_WIN //Remove when Other tab has options on Windows
    tabWidget->addTab(otherWidget, "Other");
#endif

    tabWidget->setCurrentIndex(activeTab);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                     Qt::Horizontal, this);

    layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);

    connect(cen64Button, SIGNAL(clicked()), this, SLOT(browseCen64()));
    connect(pifButton, SIGNAL(clicked()), this, SLOT(browsePIF()));
    connect(romButton, SIGNAL(clicked()), this, SLOT(browseROM()));
    connect(catalogButton, SIGNAL(clicked()), this, SLOT(browseCatalog()));
    connect(savesButton, SIGNAL(clicked()), this, SLOT(browseSaves()));
    connect(eepromButton, SIGNAL(clicked()), this, SLOT(browseEEPROM()));
    connect(sramButton, SIGNAL(clicked()), this, SLOT(browseSRAM()));
    connect(saveOption, SIGNAL(toggled(bool)), this, SLOT(toggleSaves(bool)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeColumn()));
    connect(sortUpButton, SIGNAL(clicked()), this, SLOT(sortUp()));
    connect(sortDownButton, SIGNAL(clicked()), this, SLOT(sortDown()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


void SettingsDialog::addColumn()
{
    int row = availableList->currentRow();

    if (row >= 0) {
        currentList->addItem(availableList->currentItem()->text());
        delete availableList->takeItem(row);
    }
}


void SettingsDialog::browseCatalog()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        catalogPath->setText(path);
}



void SettingsDialog::browseCen64()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        cen64Path->setText(path);
}


void SettingsDialog::browseEEPROM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        eepromPath->setText(path);
}


void SettingsDialog::browsePIF()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        pifPath->setText(path);
}


void SettingsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        romPath->setText(path);
}


void SettingsDialog::browseSaves()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        savesPath->setText(path);
}


void SettingsDialog::browseSRAM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        sramPath->setText(path);
}


void SettingsDialog::editSettings()
{
    SETTINGS.setValue("Paths/cen64", cen64Path->text());
    SETTINGS.setValue("Paths/pifrom", pifPath->text());
    SETTINGS.setValue("Paths/roms", romPath->text());
    SETTINGS.setValue("Paths/catalog", catalogPath->text());

    SETTINGS.setValue("Saves/directory", savesPath->text());
    SETTINGS.setValue("Saves/eeprom", eepromPath->text());
    SETTINGS.setValue("Saves/sram", sramPath->text());

    if (saveOption->isChecked())
        SETTINGS.setValue("Saves/individualsave", true);
    else
        SETTINGS.setValue("Saves/individualsave", "");

    QStringList visibleItems;
    foreach (QListWidgetItem *item, currentList->findItems("*", Qt::MatchWildcard))
        visibleItems << item->text();

    SETTINGS.setValue("ROMs/columns", visibleItems.join("|"));

    if (stretchOption->isChecked())
        SETTINGS.setValue("ROMs/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("ROMs/stretchfirstcolumn", "");

    if (outputOption->isChecked())
        SETTINGS.setValue("Other/consoleoutput", true);
    else
        SETTINGS.setValue("Other/consoleoutput", "");

    close();
}


void SettingsDialog::removeColumn()
{
    int row = currentList->currentRow();

    if (row >= 0) {
        availableList->addItem(currentList->currentItem()->text());
        delete currentList->takeItem(row);

        availableList->sortItems();
    }
}


void SettingsDialog::sortDown()
{
    int row = currentList->currentRow();

    if (row > 0) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row - 1, item);
        currentList->setCurrentRow(row - 1);
    }
}


void SettingsDialog::sortUp()
{
    int row = currentList->currentRow();

    if (row >= 0 && row < currentList->count() - 1) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row + 1, item);
        currentList->setCurrentRow(row + 1);
    }
}

void SettingsDialog::toggleSaves(bool active)
{
    foreach (QWidget *next, saveEnable)
        next->setEnabled(active);

    foreach (QWidget *next, saveDisable)
        next->setEnabled(!active);
}
