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
#include "ui_settingsdialog.h"
#include "global.h"


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(activeTab);


    //Populate Paths tab
    ui->cen64Path->setText(SETTINGS.value("Paths/cen64", "").toString());
    ui->pifPath->setText(SETTINGS.value("Paths/pifrom", "").toString());
    ui->romPath->setText(SETTINGS.value("Paths/roms", "").toString());
    ui->catalogPath->setText(SETTINGS.value("Paths/catalog", "").toString());

    ui->savesPath->setText(SETTINGS.value("Saves/directory", "").toString());
    ui->eepromPath->setText(SETTINGS.value("Saves/eeprom", "").toString());
    ui->sramPath->setText(SETTINGS.value("Saves/sram", "").toString());

    //Widgets enabled when save checkbox is active
    saveEnable << ui->eepromPathLabel
               << ui->eepromPath
               << ui->eepromButton
               << ui->sramPathLabel
               << ui->sramPath
               << ui->sramButton;

    //Widgets disabled when save checkbox is active
    saveDisable << ui->savesPathLabel
                << ui->savesPath
                << ui->savesButton;

    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
        toggleSaves(true);
        ui->saveOption->setChecked(true);
    } else
        toggleSaves(false);

    connect(ui->cen64Button, SIGNAL(clicked()), this, SLOT(browseCen64()));
    connect(ui->pifButton, SIGNAL(clicked()), this, SLOT(browsePIF()));
    connect(ui->romButton, SIGNAL(clicked()), this, SLOT(browseROM()));
    connect(ui->catalogButton, SIGNAL(clicked()), this, SLOT(browseCatalog()));
    connect(ui->savesButton, SIGNAL(clicked()), this, SLOT(browseSaves()));
    connect(ui->eepromButton, SIGNAL(clicked()), this, SLOT(browseEEPROM()));
    connect(ui->sramButton, SIGNAL(clicked()), this, SLOT(browseSRAM()));
    connect(ui->saveOption, SIGNAL(toggled(bool)), this, SLOT(toggleSaves(bool)));


    //Populate Columns tab
    QStringList current;
    current = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");
    populateAvailable();

    foreach (QString cur, current)
    {
        if (available.contains(cur))
            available.removeOne(cur);
        else //Someone added an invalid item
            current.removeOne(cur);
    }

    ui->availableList->addItems(available);
    ui->availableList->sortItems();

    ui->currentList->addItems(current);

    if (SETTINGS.value("ROMs/stretchfirstcolumn", "true").toString() == "true")
        ui->stretchOption->setChecked(true);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeColumn()));
    connect(ui->sortUpButton, SIGNAL(clicked()), this, SLOT(sortUp()));
    connect(ui->sortDownButton, SIGNAL(clicked()), this, SLOT(sortDown()));


    //Populate Other tab
    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        ui->downloadOption->setChecked(true);

    //Disable download option if no catalog file set
    if (SETTINGS.value("Paths/catalog", "").toString() == "") {
        ui->downloadOption->setChecked(false);
        ui->downloadOption->setEnabled(false);
        ui->downloadLabel->setEnabled(false);
    }

    if (SETTINGS.value("Other/consoleoutput", "").toString() == "true")
        ui->outputOption->setChecked(true);

#ifdef Q_OS_WIN //Remove when Other tab has options on Windows
    ui->outputLabel->setVisible(false);
    ui->outputOption->setVisible(false);
#endif


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}



void SettingsDialog::addColumn()
{
    int row = ui->availableList->currentRow();

    if (row >= 0) {
        ui->currentList->addItem(ui->availableList->currentItem()->text());
        delete ui->availableList->takeItem(row);
    }
}


void SettingsDialog::browseCatalog()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "") {
        ui->catalogPath->setText(path);

        ui->downloadOption->setEnabled(true);
        ui->downloadLabel->setEnabled(true);
    }
}



void SettingsDialog::browseCen64()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        ui->cen64Path->setText(path);
}


void SettingsDialog::browseEEPROM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        ui->eepromPath->setText(path);
}


void SettingsDialog::browsePIF()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        ui->pifPath->setText(path);
}


void SettingsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->romPath->setText(path);
}


void SettingsDialog::browseSaves()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->savesPath->setText(path);
}


void SettingsDialog::browseSRAM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        ui->sramPath->setText(path);
}


void SettingsDialog::editSettings()
{
    SETTINGS.setValue("Paths/cen64", ui->cen64Path->text());
    SETTINGS.setValue("Paths/pifrom", ui->pifPath->text());
    SETTINGS.setValue("Paths/roms", ui->romPath->text());
    SETTINGS.setValue("Paths/catalog", ui->catalogPath->text());

    SETTINGS.setValue("Saves/directory", ui->savesPath->text());
    SETTINGS.setValue("Saves/eeprom", ui->eepromPath->text());
    SETTINGS.setValue("Saves/sram", ui->sramPath->text());

    if (ui->saveOption->isChecked())
        SETTINGS.setValue("Saves/individualsave", true);
    else
        SETTINGS.setValue("Saves/individualsave", "");

    if (ui->downloadOption->isChecked() && ui->catalogPath->text() != "")
        SETTINGS.setValue("Other/downloadinfo", true);
    else
        SETTINGS.setValue("Other/downloadinfo", "");

    populateAvailable(); //This removes thegamesdb.net options if user unselects downloadOption

    QStringList visibleItems;
    foreach (QListWidgetItem *item, ui->currentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->text()))
            visibleItems << item->text();

    SETTINGS.setValue("ROMs/columns", visibleItems.join("|"));

    if (ui->stretchOption->isChecked())
        SETTINGS.setValue("ROMs/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("ROMs/stretchfirstcolumn", "");

#ifndef Q_OS_WIN
    if (ui->outputOption->isChecked())
        SETTINGS.setValue("Other/consoleoutput", true);
    else
        SETTINGS.setValue("Other/consoleoutput", "");
#endif

    close();
}


void SettingsDialog::populateAvailable() {
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

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        available << "Game Title"
                  << "Release Date"
                  << "Overview"
                  << "ESRB"
                  << "Genre"
                  << "Publisher"
                  << "Developer"
                  << "Rating";
}


void SettingsDialog::removeColumn()
{
    int row = ui->currentList->currentRow();

    if (row >= 0) {
        ui->availableList->addItem(ui->currentList->currentItem()->text());
        delete ui->currentList->takeItem(row);

        ui->availableList->sortItems();
    }
}


void SettingsDialog::sortDown()
{
    int row = ui->currentList->currentRow();

    if (row > 0) {
        QListWidgetItem *item = ui->currentList->takeItem(row);
        ui->currentList->insertItem(row - 1, item);
        ui->currentList->setCurrentRow(row - 1);
    }
}


void SettingsDialog::sortUp()
{
    int row = ui->currentList->currentRow();

    if (row >= 0 && row < ui->currentList->count() - 1) {
        QListWidgetItem *item = ui->currentList->takeItem(row);
        ui->currentList->insertItem(row + 1, item);
        ui->currentList->setCurrentRow(row + 1);
    }
}

void SettingsDialog::toggleSaves(bool active)
{
    foreach (QWidget *next, saveEnable)
        next->setEnabled(active);

    foreach (QWidget *next, saveDisable)
        next->setEnabled(!active);
}
