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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(activeTab);


    //Populate Paths tab
    ui->cen64Path->setText(SETTINGS.value("Paths/cen64", "").toString());
    ui->pifPath->setText(SETTINGS.value("Paths/pifrom", "").toString());
    ui->ddPath->setText(SETTINGS.value("Paths/ddiplrom", "").toString());
    ui->catalogPath->setText(SETTINGS.value("Paths/catalog", "").toString());

    QStringList romDirectories = SETTINGS.value("Paths/roms", "").toString().split("|");
    romDirectories.removeAll("");
    foreach (QString directory, romDirectories)
        ui->romList->addItem(directory);

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
    connect(ui->ddButton, SIGNAL(clicked()), this, SLOT(browse64DD()));
    connect(ui->catalogButton, SIGNAL(clicked()), this, SLOT(browseCatalog()));
    connect(ui->romAddButton, SIGNAL(clicked()), this, SLOT(addRomDirectory()));
    connect(ui->romRemoveButton, SIGNAL(clicked()), this, SLOT(removeRomDirectory()));
    connect(ui->savesButton, SIGNAL(clicked()), this, SLOT(browseSaves()));
    connect(ui->eepromButton, SIGNAL(clicked()), this, SLOT(browseEEPROM()));
    connect(ui->sramButton, SIGNAL(clicked()), this, SLOT(browseSRAM()));
    connect(ui->saveOption, SIGNAL(toggled(bool)), this, SLOT(toggleSaves(bool)));


    //Populate Table tab
    QStringList sizes;
    sizes << "Extra Small"
          << "Small"
          << "Medium"
          << "Large"
          << "Extra Large";

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        populateTableAndListTab(true);
    else
        populateTableAndListTab(false);

    if (SETTINGS.value("Table/stretchfirstcolumn", "true").toString() == "true")
        ui->tableStretchOption->setChecked(true);

    ui->tableSizeBox->insertItems(0, sizes);
    int tableSizeIndex = sizes.indexOf(SETTINGS.value("Table/imagesize","Medium").toString());
    if (tableSizeIndex >= 0) ui->tableSizeBox->setCurrentIndex(tableSizeIndex);

    connect(ui->tableAddButton, SIGNAL(clicked()), this, SLOT(tableAddColumn()));
    connect(ui->tableRemoveButton, SIGNAL(clicked()), this, SLOT(tableRemoveColumn()));
    connect(ui->tableSortUpButton, SIGNAL(clicked()), this, SLOT(tableSortUp()));
    connect(ui->tableSortDownButton, SIGNAL(clicked()), this, SLOT(tableSortDown()));


    //Populate Grid tab
    QStringList colors;
    colors << "Black"
           << "White"
           << "Light Gray"
           << "Dark Gray"
           << "Green"
           << "Cyan"
           << "Blue"
           << "Purple"
           << "Red"
           << "Pink"
           << "Orange"
           << "Yellow"
           << "Brown";

    ui->gridSizeBox->insertItems(0, sizes);
    int gridSizeIndex = sizes.indexOf(SETTINGS.value("Grid/imagesize","Medium").toString());
    if (gridSizeIndex >= 0) ui->gridSizeBox->setCurrentIndex(gridSizeIndex);

    int gridColumnCount = SETTINGS.value("Grid/columncount","4").toInt();
    ui->columnCountBox->setValue(gridColumnCount);

    ui->shadowActiveBox->insertItems(0, colors);
    int activeIndex = colors.indexOf(SETTINGS.value("Grid/activecolor","Cyan").toString());
    if (activeIndex >= 0) ui->shadowActiveBox->setCurrentIndex(activeIndex);

    ui->shadowInactiveBox->insertItems(0, colors);
    int inactiveIndex = colors.indexOf(SETTINGS.value("Grid/inactivecolor","Black").toString());
    if (inactiveIndex >= 0) ui->shadowInactiveBox->setCurrentIndex(inactiveIndex);

    //Widgets to enable when label active
    labelEnable << ui->labelTextLabel
                << ui->labelTextBox
                << ui->labelColorLabel
                << ui->labelColorBox;

    if (SETTINGS.value("Grid/label", "true").toString() == "true") {
        toggleLabel(true);
        ui->labelOption->setChecked(true);
    } else
        toggleLabel(false);

    ui->labelColorBox->insertItems(0, colors);
    int labelColorIndex = colors.indexOf(SETTINGS.value("Grid/labelcolor","White").toString());
    if (labelColorIndex >= 0) ui->labelColorBox->setCurrentIndex(labelColorIndex);

    ui->backgroundPath->setText(SETTINGS.value("Grid/background", "").toString());

    if (SETTINGS.value("Grid/sortdirection", "ascending").toString() == "descending")
        ui->gridDescendingOption->setChecked(true);

    connect(ui->backgroundButton, SIGNAL(clicked()), this, SLOT(browseBackground()));
    connect(ui->labelOption, SIGNAL(toggled(bool)), this, SLOT(toggleLabel(bool)));


    //Populate List tab
    listCoverEnable << ui->listSizeLabel
                    << ui->listSizeBox;

    if (SETTINGS.value("List/displaycover", "").toString() == "true") {
        toggleListCover(true);
        ui->listCoverOption->setChecked(true);
    } else
        toggleListCover(false);

    if (SETTINGS.value("List/firstitemheader", "true").toString() == "true")
        ui->listHeaderOption->setChecked(true);

    ui->listSizeBox->insertItems(0, sizes);
    int listSizeIndex = sizes.indexOf(SETTINGS.value("List/imagesize","Medium").toString());
    if (listSizeIndex >= 0) ui->listSizeBox->setCurrentIndex(listSizeIndex);

    if (SETTINGS.value("List/sortdirection", "ascending").toString() == "descending")
        ui->listDescendingOption->setChecked(true);


    connect(ui->listCoverOption, SIGNAL(toggled(bool)), this, SLOT(toggleListCover(bool)));
    connect(ui->listAddButton, SIGNAL(clicked()), this, SLOT(listAddColumn()));
    connect(ui->listRemoveButton, SIGNAL(clicked()), this, SLOT(listRemoveColumn()));
    connect(ui->listSortUpButton, SIGNAL(clicked()), this, SLOT(listSortUp()));
    connect(ui->listSortDownButton, SIGNAL(clicked()), this, SLOT(listSortDown()));


    //Populate Other tab
    downloadEnable << ui->tableSizeLabel
                   << ui->tableSizeBox
                   << ui->listCoverOption
                   << ui->listSizeLabel
                   << ui->listSizeBox;

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        toggleDownload(true);
        ui->downloadOption->setChecked(true);
    } else
        toggleDownload(false);

    if (SETTINGS.value("Other/consoleoutput", "").toString() == "true")
        ui->outputOption->setChecked(true);

#ifdef Q_OS_WIN //Remove when Other tab has options on Windows
    ui->outputLabel->setVisible(false);
    ui->outputOption->setVisible(false);
#endif

    ui->parametersLine->setText(SETTINGS.value("Other/parameters", "").toString());

    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(toggleDownload(bool)));
    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(populateTableAndListTab(bool)));


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}



void SettingsDialog::addColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = availableList->currentRow();

    if (row >= 0) {
        currentList->addItem(availableList->currentItem()->text());
        delete availableList->takeItem(row);
    }
}


void SettingsDialog::addRomDirectory()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("ROM Directory"));
    if (path != "") {
        //check for duplicates
        bool found = false;
        foreach (QListWidgetItem *item, ui->romList->findItems("*", Qt::MatchWildcard))
            if (path == item->text())
                found = true;

        if (!found)
            ui->romList->addItem(path);
    }
}


void SettingsDialog::browse64DD()
{
    QString path = QFileDialog::getOpenFileName(this, tr("64DD IPL ROM File"));
    if (path != "")
        ui->ddPath->setText(path);
}


void SettingsDialog::browseBackground()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Background Image"));
    if (path != "")
        ui->backgroundPath->setText(path);
}


void SettingsDialog::browseCatalog()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Catalog File"));
    if (path != "")
        ui->catalogPath->setText(path);
}



void SettingsDialog::browseCen64()
{
    QString path = QFileDialog::getOpenFileName(this, tr("CEN64 Executable"));
    if (path != "")
        ui->cen64Path->setText(path);
}


void SettingsDialog::browseEEPROM()
{
    QString path = QFileDialog::getOpenFileName(this, tr("EEPROM File"));
    if (path != "")
        ui->eepromPath->setText(path);
}


void SettingsDialog::browsePIF()
{
    QString path = QFileDialog::getOpenFileName(this, tr("PIF IPL ROM File"));
    if (path != "")
        ui->pifPath->setText(path);
}


void SettingsDialog::browseSaves()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Save Directory"));
    if (path != "")
        ui->savesPath->setText(path);
}


void SettingsDialog::browseSRAM()
{
    QString path = QFileDialog::getOpenFileName(this, tr("SRAM File"));
    if (path != "")
        ui->sramPath->setText(path);
}


void SettingsDialog::editSettings()
{
    //Set download option first
    if (ui->downloadOption->isChecked()) {
        SETTINGS.setValue("Other/downloadinfo", true);
        populateAvailable(true); //This removes thegamesdb.net options if user unselects download
    } else {
        SETTINGS.setValue("Other/downloadinfo", "");
        populateAvailable(false);
    }


    //Paths tab
    SETTINGS.setValue("Paths/cen64", ui->cen64Path->text());
    SETTINGS.setValue("Paths/pifrom", ui->pifPath->text());
    SETTINGS.setValue("Paths/ddiplrom", ui->ddPath->text());
    SETTINGS.setValue("Paths/catalog", ui->catalogPath->text());

    SETTINGS.setValue("Saves/directory", ui->savesPath->text());
    SETTINGS.setValue("Saves/eeprom", ui->eepromPath->text());
    SETTINGS.setValue("Saves/sram", ui->sramPath->text());

    QStringList romDirectories;
    foreach (QListWidgetItem *item, ui->romList->findItems("*", Qt::MatchWildcard))
        romDirectories << item->text();

    SETTINGS.setValue("Paths/roms", romDirectories.join("|"));

    if (ui->saveOption->isChecked())
        SETTINGS.setValue("Saves/individualsave", true);
    else
        SETTINGS.setValue("Saves/individualsave", "");


    //Table tab
    QStringList tableVisibleItems;
    foreach (QListWidgetItem *item, ui->tableCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->text()))
            tableVisibleItems << item->text();

    SETTINGS.setValue("Table/columns", tableVisibleItems.join("|"));

    if (ui->tableStretchOption->isChecked())
        SETTINGS.setValue("Table/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("Table/stretchfirstcolumn", "");

    SETTINGS.setValue("Table/imagesize", ui->tableSizeBox->currentText());


    //Grid tab
    SETTINGS.setValue("Grid/imagesize", ui->gridSizeBox->currentText());
    SETTINGS.setValue("Grid/columncount", ui->columnCountBox->value());
    SETTINGS.setValue("Grid/inactivecolor", ui->shadowInactiveBox->currentText());
    SETTINGS.setValue("Grid/activecolor", ui->shadowActiveBox->currentText());
    SETTINGS.setValue("Grid/background", ui->backgroundPath->text());

    if (ui->labelOption->isChecked())
        SETTINGS.setValue("Grid/label", true);
    else
        SETTINGS.setValue("Grid/label", "");

    SETTINGS.setValue("Grid/labeltext", ui->labelTextBox->currentText());
    SETTINGS.setValue("Grid/labelcolor", ui->labelColorBox->currentText());
    SETTINGS.setValue("Grid/sort", ui->gridSortBox->currentText());

    if (ui->gridDescendingOption->isChecked())
        SETTINGS.setValue("Grid/sortdirection", "descending");
    else
        SETTINGS.setValue("Grid/sortdirection", "ascending");


    //List tab
    QStringList listVisibleItems;
    foreach (QListWidgetItem *item, ui->listCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->text()))
            listVisibleItems << item->text();

    SETTINGS.setValue("List/columns", listVisibleItems.join("|"));

    if (ui->listHeaderOption->isChecked())
        SETTINGS.setValue("List/firstitemheader", true);
    else
        SETTINGS.setValue("List/firstitemheader", "");

    if (ui->listCoverOption->isChecked() && ui->downloadOption->isChecked())
        SETTINGS.setValue("List/displaycover", true);
    else
        SETTINGS.setValue("List/displaycover", "");

    SETTINGS.setValue("List/imagesize", ui->listSizeBox->currentText());
    SETTINGS.setValue("List/sort", ui->listSortBox->currentText());

    if (ui->listDescendingOption->isChecked())
        SETTINGS.setValue("List/sortdirection", "descending");
    else
        SETTINGS.setValue("List/sortdirection", "ascending");


    //Other tab
#ifndef Q_OS_WIN
    if (ui->outputOption->isChecked())
        SETTINGS.setValue("Other/consoleoutput", true);
    else
        SETTINGS.setValue("Other/consoleoutput", "");
#endif

    SETTINGS.setValue("Other/parameters", ui->parametersLine->text());

    close();
}


void SettingsDialog::listAddColumn()
{
    addColumn(ui->listCurrentList, ui->listAvailableList);
}


void SettingsDialog::listRemoveColumn()
{
    removeColumn(ui->listCurrentList, ui->listAvailableList);
}


void SettingsDialog::listSortDown()
{
    sortDown(ui->listCurrentList);
}


void SettingsDialog::listSortUp()
{
    sortUp(ui->listCurrentList);
}


void SettingsDialog::populateAvailable(bool downloadItems) {
    available.clear();
    labelOptions.clear();
    sortOptions.clear();

    available << "Filename"
              << "Filename (extension)"
              << "Zip File"
              << "GoodName"
              << "Internal Name"
              << "Size"
              << "MD5"
              << "CRC1"
              << "CRC2"
              << "Players"
              << "Rumble"
              << "Save Type";

    labelOptions << "Filename"
                 << "Filename (extension)"
                 << "GoodName"
                 << "Internal Name";

    sortOptions << "Filename"
                << "GoodName"
                << "Internal Name"
                << "Size";

    if (downloadItems) {
        available << "Game Title"
                  << "Release Date"
                  << "Overview"
                  << "ESRB"
                  << "Genre"
                  << "Publisher"
                  << "Developer"
                  << "Rating"
                  << "Game Cover";

        labelOptions << "Game Title"
                     << "Release Date"
                     << "Genre";

        sortOptions << "Game Title"
                    << "Release Date"
                    << "ESRB"
                    << "Genre"
                    << "Publisher"
                    << "Developer"
                    << "Rating";
    }

    available.sort();
    labelOptions.sort();
    sortOptions.sort();
}


void SettingsDialog::populateTableAndListTab(bool downloadItems)
{
    populateAvailable(downloadItems);

    //Table columns and sort fields
    QStringList tableCurrent, tableAvailable;
    tableCurrent = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");
    tableAvailable = available;

    foreach (QString cur, tableCurrent)
    {
        if (tableAvailable.contains(cur))
            tableAvailable.removeOne(cur);
        else //Someone added an invalid item
            tableCurrent.removeOne(cur);
    }

    ui->tableAvailableList->clear();
    ui->tableAvailableList->addItems(tableAvailable);
    ui->tableAvailableList->sortItems();

    ui->tableCurrentList->clear();
    ui->tableCurrentList->addItems(tableCurrent);


    //Grid sort field and label text
    ui->labelTextBox->clear();
    ui->labelTextBox->insertItems(0, labelOptions);
    int labelTextIndex = labelOptions.indexOf(SETTINGS.value("Grid/labeltext","Filename").toString());
    if (labelTextIndex >= 0) ui->labelTextBox->setCurrentIndex(labelTextIndex);

    ui->gridSortBox->clear();
    ui->gridSortBox->insertItems(0, sortOptions);
    int gridSortIndex = sortOptions.indexOf(SETTINGS.value("Grid/sort","Filename").toString());
    if (gridSortIndex >= 0) ui->gridSortBox->setCurrentIndex(gridSortIndex);


    //List items and sort field
    QStringList listCurrent, listAvailable;
    listCurrent = SETTINGS.value("List/columns", "Filename|Internal Name|Size").toString().split("|");
    listAvailable = available;
    listAvailable.removeOne("Game Cover"); //Game Cover handled separately

    foreach (QString cur, listCurrent)
    {
        if (listAvailable.contains(cur))
            listAvailable.removeOne(cur);
        else //Someone added an invalid item
            listCurrent.removeOne(cur);
    }

    ui->listAvailableList->clear();
    ui->listAvailableList->addItems(listAvailable);
    ui->listAvailableList->sortItems();

    ui->listCurrentList->clear();
    ui->listCurrentList->addItems(listCurrent);

    ui->listSortBox->clear();
    ui->listSortBox->insertItems(0, sortOptions);
    int listSortIndex = sortOptions.indexOf(SETTINGS.value("List/sort","Filename").toString());
    if (listSortIndex >= 0) ui->listSortBox->setCurrentIndex(listSortIndex);
}


void SettingsDialog::removeColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = currentList->currentRow();

    if (row >= 0) {
        availableList->addItem(currentList->currentItem()->text());
        delete currentList->takeItem(row);

        availableList->sortItems();
    }
}


void SettingsDialog::removeRomDirectory()
{
    int row = ui->romList->currentRow();

    if (row >= 0)
        delete ui->romList->takeItem(row);
}


void SettingsDialog::sortDown(QListWidget *currentList)
{
    int row = currentList->currentRow();

    if (row > 0) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row - 1, item);
        currentList->setCurrentRow(row - 1);
    }
}


void SettingsDialog::sortUp(QListWidget *currentList)
{
    int row = currentList->currentRow();

    if (row >= 0 && row < currentList->count() - 1) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row + 1, item);
        currentList->setCurrentRow(row + 1);
    }
}


void SettingsDialog::tableAddColumn()
{
    addColumn(ui->tableCurrentList, ui->tableAvailableList);
}


void SettingsDialog::tableRemoveColumn()
{
    removeColumn(ui->tableCurrentList, ui->tableAvailableList);
}


void SettingsDialog::tableSortDown()
{
    sortDown(ui->tableCurrentList);
}


void SettingsDialog::tableSortUp()
{
    sortUp(ui->tableCurrentList);
}


void SettingsDialog::toggleDownload(bool active)
{
    foreach (QWidget *next, downloadEnable)
        next->setEnabled(active);

    if (active)
        toggleListCover(ui->listCoverOption->isChecked());
}


void SettingsDialog::toggleLabel(bool active)
{
    foreach (QWidget *next, labelEnable)
        next->setEnabled(active);
}


void SettingsDialog::toggleListCover(bool active)
{
    foreach (QWidget *next, listCoverEnable)
        next->setEnabled(active);
}


void SettingsDialog::toggleSaves(bool active)
{
    foreach (QWidget *next, saveEnable)
        next->setEnabled(active);

    foreach (QWidget *next, saveDisable)
        next->setEnabled(!active);
}
