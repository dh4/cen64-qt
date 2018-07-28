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

#include "../global.h"
#include "../common.h"

#include <QFileDialog>
#include <QListWidget>
#include <QTranslator>


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(activeTab);


    //Populate Paths tab
    ui->emulatorPath->setText(SETTINGS.value("Paths/cen64", "").toString());
    ui->pifPath->setText(SETTINGS.value("Paths/pifrom", "").toString());
    ui->ddPath->setText(SETTINGS.value("Paths/ddiplrom", "").toString());
    ui->catalogPath->setText(SETTINGS.value("Paths/catalog", "").toString());

    QStringList romDirectories = SETTINGS.value("Paths/roms", "").toString().split("|");
    romDirectories.removeAll("");
    foreach (QString directory, romDirectories)
        ui->romList->addItem(directory);

    ui->savesPath->setText(SETTINGS.value("Saves/directory", "").toString());
    ui->eeprom4kPath->setText(SETTINGS.value("Saves/eeprom4k", "").toString());
    ui->eeprom16kPath->setText(SETTINGS.value("Saves/eeprom16k", "").toString());
    ui->sramPath->setText(SETTINGS.value("Saves/sram", "").toString());
    ui->flashPath->setText(SETTINGS.value("Saves/flash", "").toString());

    //Widgets enabled when save checkbox is active
    saveEnable << ui->eeprom4kPathLabel
               << ui->eeprom4kPath
               << ui->eeprom4kButton
               << ui->eeprom16kPathLabel
               << ui->eeprom16kPath
               << ui->eeprom16kButton
               << ui->sramPathLabel
               << ui->sramPath
               << ui->sramButton
               << ui->flashPathLabel
               << ui->flashPath
               << ui->flashButton;

    //Widgets disabled when save checkbox is active
    saveDisable << ui->savesPathLabel
                << ui->savesPath
                << ui->savesButton;

    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
        toggleSaves(true);
        ui->saveOption->setChecked(true);
    } else
        toggleSaves(false);

    connect(ui->emulatorButton, SIGNAL(clicked()), this, SLOT(browseEmulator()));
    connect(ui->pifButton, SIGNAL(clicked()), this, SLOT(browsePIF()));
    connect(ui->ddButton, SIGNAL(clicked()), this, SLOT(browse64DD()));
    connect(ui->catalogButton, SIGNAL(clicked()), this, SLOT(browseCatalog()));
    connect(ui->romAddButton, SIGNAL(clicked()), this, SLOT(addRomDirectory()));
    connect(ui->romRemoveButton, SIGNAL(clicked()), this, SLOT(removeRomDirectory()));
    connect(ui->savesButton, SIGNAL(clicked()), this, SLOT(browseSaves()));
    connect(ui->eeprom4kButton, SIGNAL(clicked()), this, SLOT(browseEEPROM4k()));
    connect(ui->eeprom16kButton, SIGNAL(clicked()), this, SLOT(browseEEPROM16k()));
    connect(ui->sramButton, SIGNAL(clicked()), this, SLOT(browseSRAM()));
    connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(browseFlashRAM()));
    connect(ui->saveOption, SIGNAL(toggled(bool)), this, SLOT(toggleSaves(bool)));


    //Populate Emulation tab
    if (SETTINGS.value("Emulation/multithread", "").toString() == "true")
        ui->multithreadOption->setChecked(true);
    if (SETTINGS.value("Emulation/noaudio", "").toString() == "true")
        ui->noAudioOption->setChecked(true);
    if (SETTINGS.value("Emulation/novideo", "").toString() == "true")
        ui->noVideoOption->setChecked(true);


    //Populate Controllers tab
    ctrlEnabled << ui->ctrl1Enabled
                << ui->ctrl2Enabled
                << ui->ctrl3Enabled
                << ui->ctrl4Enabled;

    ctrlAccessory << ui->ctrl1Accessory
                  << ui->ctrl2Accessory
                  << ui->ctrl3Accessory
                  << ui->ctrl4Accessory;

    ctrlAccessoryLabel << ui->ctrl1AccessoryLabel
                       << ui->ctrl2AccessoryLabel
                       << ui->ctrl3AccessoryLabel
                       << ui->ctrl4AccessoryLabel;

    ctrlMemPak << ui->ctrl1MemPak
               << ui->ctrl2MemPak
               << ui->ctrl3MemPak
               << ui->ctrl4MemPak;

    ctrlMemPakButton << ui->ctrl1MemPakButton
                     << ui->ctrl2MemPakButton
                     << ui->ctrl3MemPakButton
                     << ui->ctrl4MemPakButton;

    ctrlMemPakLabel << ui->ctrl1MemPakLabel
                    << ui->ctrl2MemPakLabel
                    << ui->ctrl3MemPakLabel
                    << ui->ctrl4MemPakLabel;

    ctrlTPakROM << ui->ctrl1TPakROM
                << ui->ctrl2TPakROM
                << ui->ctrl3TPakROM
                << ui->ctrl4TPakROM;

    ctrlTPakROMButton << ui->ctrl1TPakROMButton
                      << ui->ctrl2TPakROMButton
                      << ui->ctrl3TPakROMButton
                      << ui->ctrl4TPakROMButton;

    ctrlTPakROMLabel << ui->ctrl1TPakROMLabel
                     << ui->ctrl2TPakROMLabel
                     << ui->ctrl3TPakROMLabel
                     << ui->ctrl4TPakROMLabel;

    ctrlTPakSave << ui->ctrl1TPakSave
                 << ui->ctrl2TPakSave
                 << ui->ctrl3TPakSave
                 << ui->ctrl4TPakSave;

    ctrlTPakSaveButton << ui->ctrl1TPakSaveButton
                       << ui->ctrl2TPakSaveButton
                       << ui->ctrl3TPakSaveButton
                       << ui->ctrl4TPakSaveButton;

    ctrlTPakSaveLabel << ui->ctrl1TPakSaveLabel
                      << ui->ctrl2TPakSaveLabel
                      << ui->ctrl3TPakSaveLabel
                      << ui->ctrl4TPakSaveLabel;

    QStringList ctrlOptions;
    ctrlOptions << tr("None") << tr("Rumble Pak") << tr("Controller Pak") << tr("Transfer Pak");

    for (int i = 0; i <= 3; i++)
    {
        QString ctrl = "Controller"+ QString::number(i + 1);

        ctrlAccessory.at(i)->insertItems(0, ctrlOptions);
        int ctrlAccessoryIndex = SETTINGS.value(ctrl+"/accessory", 0).toInt();
        if (ctrlAccessoryIndex >= 0) ctrlAccessory.at(i)->setCurrentIndex(ctrlAccessoryIndex);
        toggleAccessory(ctrlAccessory.at(i)->currentIndex(), i);

        ctrlMemPak.at(i)->setText(SETTINGS.value(ctrl+"/mempak", "").toString());
        ctrlTPakROM.at(i)->setText(SETTINGS.value(ctrl+"/tpakrom", "").toString());
        ctrlTPakSave.at(i)->setText(SETTINGS.value(ctrl+"/tpaksave", "").toString());

        if (SETTINGS.value(ctrl+"/enabled", "").toString() == "true") {
            ctrlEnabled.at(i)->setChecked(true);
            toggleController(true, i);
        } else
            toggleController(false, i);

        connect(ctrlEnabled.at(i), SIGNAL(toggled(bool)), this, SLOT(toggleController(bool)));
        connect(ctrlAccessory.at(i), SIGNAL(currentIndexChanged(int)), this, SLOT(toggleAccessory(int)));
        connect(ctrlMemPakButton.at(i), SIGNAL(clicked()), this, SLOT(browseMemPak()));
        connect(ctrlTPakROMButton.at(i), SIGNAL(clicked()), this, SLOT(browseTPakROM()));
        connect(ctrlTPakSaveButton.at(i), SIGNAL(clicked()), this, SLOT(browseTPakSave()));
    }


    //Populate Table tab
    int tableSizeIndex = 0;
    QString currentTableSize = SETTINGS.value("Table/imagesize","Medium").toString();

    QList<QStringList> sizes;
    sizes << (QStringList() << tr("Extra Small") << "Extra Small")
          << (QStringList() << tr("Small")       << "Small")
          << (QStringList() << tr("Medium")      << "Medium")
          << (QStringList() << tr("Large")       << "Large")
          << (QStringList() << tr("Extra Large") << "Extra Large")
          << (QStringList() << tr("Super")       << "Super");

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        populateTableAndListTab(true);
    else
        populateTableAndListTab(false);

    if (SETTINGS.value("Table/stretchfirstcolumn", "true").toString() == "true")
        ui->tableStretchOption->setChecked(true);

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->tableSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentTableSize == sizes.at(i).at(1))
            tableSizeIndex = i;
    }
    if (tableSizeIndex >= 0) ui->tableSizeBox->setCurrentIndex(tableSizeIndex);

    connect(ui->tableAddButton, SIGNAL(clicked()), this, SLOT(tableAddColumn()));
    connect(ui->tableRemoveButton, SIGNAL(clicked()), this, SLOT(tableRemoveColumn()));
    connect(ui->tableSortUpButton, SIGNAL(clicked()), this, SLOT(tableSortUp()));
    connect(ui->tableSortDownButton, SIGNAL(clicked()), this, SLOT(tableSortDown()));


    //Populate Grid tab
    int gridSizeIndex = 0, activeIndex = 0, inactiveIndex = 0, labelColorIndex = 0, bgThemeIndex = 0;
    QString currentGridSize = SETTINGS.value("Grid/imagesize","Medium").toString();
    QString currentActiveColor = SETTINGS.value("Grid/activecolor","Cyan").toString();
    QString currentInactiveColor = SETTINGS.value("Grid/inactivecolor","Black").toString();
    QString currentLabelColor = SETTINGS.value("Grid/labelcolor","White").toString();
    QString currentBGTheme = SETTINGS.value("Grid/theme","Normal").toString();

    QList<QStringList> colors;
    colors << (QStringList() << tr("Black")      << "Black")
           << (QStringList() << tr("White")      << "White")
           << (QStringList() << tr("Light Gray") << "Light Gray")
           << (QStringList() << tr("Dark Gray")  << "Dark Gray")
           << (QStringList() << tr("Green")      << "Green")
           << (QStringList() << tr("Cyan")       << "Cyan")
           << (QStringList() << tr("Blue")       << "Blue")
           << (QStringList() << tr("Purple")     << "Purple")
           << (QStringList() << tr("Red")        << "Red")
           << (QStringList() << tr("Pink")       << "Pink")
           << (QStringList() << tr("Orange")     << "Orange")
           << (QStringList() << tr("Yellow")     << "Yellow")
           << (QStringList() << tr("Brown")      << "Brown");

    QList<QStringList> bgThemes;
    bgThemes << (QStringList() << tr("Light")   << "Light")
             << (QStringList() << tr("Normal")  << "Normal")
             << (QStringList() << tr("Dark")    << "Dark");

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->gridSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentGridSize == sizes.at(i).at(1))
            gridSizeIndex = i;
    }
    if (gridSizeIndex >= 0) ui->gridSizeBox->setCurrentIndex(gridSizeIndex);

    int gridColumnCount = SETTINGS.value("Grid/columncount","4").toInt();
    ui->columnCountBox->setValue(gridColumnCount);

    if (SETTINGS.value("Grid/autocolumns", "true").toString() == "true") {
        toggleGridColumn(true);
        ui->autoColumnOption->setChecked(true);
    } else
        toggleGridColumn(false);

    for (int i = 0; i < colors.length(); i++)
    {
        ui->shadowActiveBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentActiveColor == colors.at(i).at(1))
            activeIndex = i;

        ui->shadowInactiveBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentInactiveColor == colors.at(i).at(1))
            inactiveIndex = i;

        ui->labelColorBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentLabelColor == colors.at(i).at(1))
            labelColorIndex = i;
    }
    if (activeIndex >= 0) ui->shadowActiveBox->setCurrentIndex(activeIndex);
    if (inactiveIndex >= 0) ui->shadowInactiveBox->setCurrentIndex(inactiveIndex);
    if (labelColorIndex >= 0) ui->labelColorBox->setCurrentIndex(labelColorIndex);

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

    for (int i = 0; i < bgThemes.length(); i++)
    {
        ui->bgThemeBox->insertItem(i, bgThemes.at(i).at(0), bgThemes.at(i).at(1));
        if (currentBGTheme == bgThemes.at(i).at(1))
            bgThemeIndex = i;
    }
    if (bgThemeIndex >= 0) ui->bgThemeBox->setCurrentIndex(bgThemeIndex);

    QString imagePath = SETTINGS.value("Grid/background", "").toString();
    ui->backgroundPath->setText(imagePath);
    hideBGTheme(imagePath);

    if (SETTINGS.value("Grid/sortdirection", "ascending").toString() == "descending")
        ui->gridDescendingOption->setChecked(true);

    connect(ui->autoColumnOption, SIGNAL(toggled(bool)), this, SLOT(toggleGridColumn(bool)));
    connect(ui->backgroundPath, SIGNAL(textChanged(QString)), this, SLOT(hideBGTheme(QString)));
    connect(ui->backgroundButton, SIGNAL(clicked()), this, SLOT(browseBackground()));
    connect(ui->labelOption, SIGNAL(toggled(bool)), this, SLOT(toggleLabel(bool)));


    //Populate List tab
    int listSizeIndex = 0, listTextIndex = 0, listThemeIndex = 0;
    QString currentListSize = SETTINGS.value("List/imagesize","Medium").toString();
    QString currentListText = SETTINGS.value("List/textsize","Medium").toString();
    QString currentListTheme = SETTINGS.value("List/theme","Light").toString();

    QList<QStringList> themes;
    themes << (QStringList() << tr("Light") << "Light")
           << (QStringList() << tr("Dark")  << "Dark");

    listCoverEnable << ui->listSizeLabel
                    << ui->listSizeBox;

    if (SETTINGS.value("List/displaycover", "").toString() == "true") {
        toggleListCover(true);
        ui->listCoverOption->setChecked(true);
    } else
        toggleListCover(false);

    if (SETTINGS.value("List/firstitemheader", "true").toString() == "true")
        ui->listHeaderOption->setChecked(true);

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->listSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentListSize == sizes.at(i).at(1))
            listSizeIndex = i;
        ui->listTextBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentListText == sizes.at(i).at(1))
            listTextIndex = i;
    }
    if (listSizeIndex >= 0) ui->listSizeBox->setCurrentIndex(listSizeIndex);
    if (listTextIndex >= 0) ui->listTextBox->setCurrentIndex(listTextIndex);

    for (int i = 0; i < themes.length(); i++)
    {
        ui->listThemeBox->insertItem(i, themes.at(i).at(0), themes.at(i).at(1));
        if (currentListTheme == themes.at(i).at(1))
            listThemeIndex = i;
    }
    if (listThemeIndex >= 0) ui->listThemeBox->setCurrentIndex(listThemeIndex);

    if (SETTINGS.value("List/sortdirection", "ascending").toString() == "descending")
        ui->listDescendingOption->setChecked(true);


    connect(ui->listCoverOption, SIGNAL(toggled(bool)), this, SLOT(toggleListCover(bool)));
    connect(ui->listAddButton, SIGNAL(clicked()), this, SLOT(listAddColumn()));
    connect(ui->listRemoveButton, SIGNAL(clicked()), this, SLOT(listRemoveColumn()));
    connect(ui->listSortUpButton, SIGNAL(clicked()), this, SLOT(listSortUp()));
    connect(ui->listSortDownButton, SIGNAL(clicked()), this, SLOT(listSortDown()));


    //Populate Other tab
    int languageIndex = 0;
    QString currentLanguage = SETTINGS.value("language", getDefaultLanguage()).toString();

    QList<QStringList> languages;
    languages << (QStringList() << QString::fromUtf8("English")  << "EN")
              << (QStringList() << QString::fromUtf8("Français") << "FR");

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

#ifdef Q_OS_WIN
    ui->outputLabel->setVisible(false);
    ui->outputOption->setVisible(false);
#endif

    ui->parametersLine->setText(SETTINGS.value("Other/parameters", "").toString());

    for (int i = 0; i < languages.length(); i++)
    {
        ui->languageBox->insertItem(i, languages.at(i).at(0), languages.at(i).at(1));
        if (currentLanguage == languages.at(i).at(1))
            languageIndex = i;
    }
    ui->languageBox->setCurrentIndex(languageIndex);

    ui->languageInfoLabel->setHidden(true);

    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(toggleDownload(bool)));
    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(populateTableAndListTab(bool)));
    connect(ui->languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLanguageInfo()));


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

    if (row >= 0)
        currentList->addItem(availableList->takeItem(row));
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


void SettingsDialog::browseEmulator()
{
    QString path = QFileDialog::getOpenFileName(this, tr("<ParentName> Executable")
                                                .replace("<ParentName>",ParentName));
    if (path != "")
        ui->emulatorPath->setText(path);
}


void SettingsDialog::browseEEPROM4k()
{
    QString path = QFileDialog::getOpenFileName(this, tr("4kbit EEPROM File"));
    if (path != "")
        ui->eeprom4kPath->setText(path);
}


void SettingsDialog::browseEEPROM16k()
{
    QString path = QFileDialog::getOpenFileName(this, tr("16kbit EEPROM File"));
    if (path != "")
        ui->eeprom16kPath->setText(path);
}


void SettingsDialog::browseFlashRAM()
{
    QString path = QFileDialog::getOpenFileName(this, tr("FlashRAM File"));
    if (path != "")
        ui->flashPath->setText(path);
}


void SettingsDialog::browseMemPak()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Controller Pak File"));
    if (path != "")
        ctrlMemPak.at(ui->controllersTabWidget->currentIndex())->setText(path);
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


void SettingsDialog::browseTPakROM()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Transfer Pak ROM File"));
    if (path != "")
        ctrlTPakROM.at(ui->controllersTabWidget->currentIndex())->setText(path);
}


void SettingsDialog::browseTPakSave()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Transfer Pak Save File"));
    if (path != "")
        ctrlTPakSave.at(ui->controllersTabWidget->currentIndex())->setText(path);
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
    SETTINGS.setValue("Paths/cen64", ui->emulatorPath->text());
    SETTINGS.setValue("Paths/pifrom", ui->pifPath->text());
    SETTINGS.setValue("Paths/ddiplrom", ui->ddPath->text());
    SETTINGS.setValue("Paths/catalog", ui->catalogPath->text());

    SETTINGS.setValue("Saves/directory", ui->savesPath->text());
    SETTINGS.setValue("Saves/eeprom4k", ui->eeprom4kPath->text());
    SETTINGS.setValue("Saves/eeprom16k", ui->eeprom16kPath->text());
    SETTINGS.setValue("Saves/sram", ui->sramPath->text());
    SETTINGS.setValue("Saves/flash", ui->flashPath->text());

    QStringList romDirectories;
    foreach (QListWidgetItem *item, ui->romList->findItems("*", Qt::MatchWildcard))
        romDirectories << item->text();

    SETTINGS.setValue("Paths/roms", romDirectories.join("|"));

    if (ui->saveOption->isChecked())
        SETTINGS.setValue("Saves/individualsave", true);
    else
        SETTINGS.setValue("Saves/individualsave", "");


    //Emulation tab
    if (ui->multithreadOption->isChecked())
        SETTINGS.setValue("Emulation/multithread", true);
    else
        SETTINGS.setValue("Emulation/multithread", "");

    if (ui->noAudioOption->isChecked())
        SETTINGS.setValue("Emulation/noaudio", true);
    else
        SETTINGS.setValue("Emulation/noaudio", "");

    if (ui->noVideoOption->isChecked())
        SETTINGS.setValue("Emulation/novideo", true);
    else
        SETTINGS.setValue("Emulation/novideo", "");


    //Controllers tab
    for (int i = 0; i <= 3; i++)
    {
        QString ctrl = "Controller"+QString::number(i + 1);

        if (ctrlEnabled.at(i)->isChecked())
            SETTINGS.setValue(ctrl+"/enabled", true);
        else
            SETTINGS.setValue(ctrl+"/enabled", "");

        SETTINGS.setValue(ctrl+"/accessory", ctrlAccessory.at(i)->currentIndex());

        SETTINGS.setValue(ctrl+"/mempak", ctrlMemPak.at(i)->text());
        SETTINGS.setValue(ctrl+"/tpakrom", ctrlTPakROM.at(i)->text());
        SETTINGS.setValue(ctrl+"/tpaksave", ctrlTPakSave.at(i)->text());
    }


    //Table tab
    QStringList tableVisibleItems;
    foreach (QListWidgetItem *item, ui->tableCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->data(Qt::UserRole).toString()))
            tableVisibleItems << item->data(Qt::UserRole).toString();

    SETTINGS.setValue("Table/columns", tableVisibleItems.join("|"));

    if (ui->tableStretchOption->isChecked())
        SETTINGS.setValue("Table/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("Table/stretchfirstcolumn", "");

    SETTINGS.setValue("Table/imagesize", ui->tableSizeBox->itemData(ui->tableSizeBox->currentIndex()));


    //Grid tab
    SETTINGS.setValue("Grid/imagesize", ui->gridSizeBox->itemData(ui->gridSizeBox->currentIndex()));
    SETTINGS.setValue("Grid/columncount", ui->columnCountBox->value());

    if (ui->autoColumnOption->isChecked())
        SETTINGS.setValue("Grid/autocolumns", true);
    else
        SETTINGS.setValue("Grid/autocolumns", "");

    SETTINGS.setValue("Grid/inactivecolor", ui->shadowInactiveBox->itemData(ui->shadowInactiveBox->currentIndex()));
    SETTINGS.setValue("Grid/activecolor", ui->shadowActiveBox->itemData(ui->shadowActiveBox->currentIndex()));
    SETTINGS.setValue("Grid/theme", ui->bgThemeBox->itemData(ui->bgThemeBox->currentIndex()));
    SETTINGS.setValue("Grid/background", ui->backgroundPath->text());

    if (ui->labelOption->isChecked())
        SETTINGS.setValue("Grid/label", true);
    else
        SETTINGS.setValue("Grid/label", "");

    SETTINGS.setValue("Grid/labeltext", ui->labelTextBox->itemData(ui->labelTextBox->currentIndex()));
    SETTINGS.setValue("Grid/labelcolor", ui->labelColorBox->itemData(ui->labelColorBox->currentIndex()));
    SETTINGS.setValue("Grid/sort", ui->gridSortBox->itemData(ui->gridSortBox->currentIndex()));

    if (ui->gridDescendingOption->isChecked())
        SETTINGS.setValue("Grid/sortdirection", "descending");
    else
        SETTINGS.setValue("Grid/sortdirection", "ascending");


    //List tab
    QStringList listVisibleItems;
    foreach (QListWidgetItem *item, ui->listCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->data(Qt::UserRole).toString()))
            listVisibleItems << item->data(Qt::UserRole).toString();

    SETTINGS.setValue("List/columns", listVisibleItems.join("|"));

    if (ui->listHeaderOption->isChecked())
        SETTINGS.setValue("List/firstitemheader", true);
    else
        SETTINGS.setValue("List/firstitemheader", "");

    if (ui->listCoverOption->isChecked() && ui->downloadOption->isChecked())
        SETTINGS.setValue("List/displaycover", true);
    else
        SETTINGS.setValue("List/displaycover", "");

    SETTINGS.setValue("List/imagesize", ui->listSizeBox->itemData(ui->listSizeBox->currentIndex()));
    SETTINGS.setValue("List/textsize", ui->listTextBox->itemData(ui->listTextBox->currentIndex()));
    SETTINGS.setValue("List/theme", ui->listThemeBox->itemData(ui->listThemeBox->currentIndex()));
    SETTINGS.setValue("List/sort", ui->listSortBox->itemData(ui->listSortBox->currentIndex()));

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
    SETTINGS.setValue("language", ui->languageBox->itemData(ui->languageBox->currentIndex()));

    close();
}


void SettingsDialog::hideBGTheme(QString imagePath)
{
    if (imagePath == "") {
        ui->bgThemeLabel->setEnabled(true);
        ui->bgThemeBox->setEnabled(true);
    } else {
        ui->bgThemeLabel->setEnabled(false);
        ui->bgThemeBox->setEnabled(false);
    }
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
                  << "Game Cover";

        labelOptions << "Game Title"
                     << "Release Date"
                     << "Genre";

        sortOptions << "Game Title"
                    << "Release Date"
                    << "ESRB"
                    << "Genre"
                    << "Publisher"
                    << "Developer";
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
    foreach (QString listItem, tableAvailable)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->tableAvailableList->addItem(item);
    }
    ui->tableAvailableList->sortItems();

    ui->tableCurrentList->clear();
    foreach (QString listItem, tableCurrent)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->tableCurrentList->addItem(item);
    }


    //Grid sort field and label text
    int labelTextIndex = 0, gridSortIndex = 0;
    QString currentLabelText = SETTINGS.value("Grid/labeltext","Filename").toString();
    QString currentGridSort = SETTINGS.value("Grid/sort","Filename").toString();

    ui->labelTextBox->clear();
    for (int i = 0; i < labelOptions.length(); i++)
    {
        ui->labelTextBox->insertItem(i, getTranslation(labelOptions.at(i)), labelOptions.at(i));
        if (currentLabelText == labelOptions.at(i))
            labelTextIndex = i;
    }
    if (labelTextIndex >= 0) ui->labelTextBox->setCurrentIndex(labelTextIndex);

    ui->gridSortBox->clear();
    for (int i = 0; i < sortOptions.length(); i++)
    {
        ui->gridSortBox->insertItem(i, getTranslation(sortOptions.at(i)), sortOptions.at(i));
        if (currentGridSort == sortOptions.at(i))
            gridSortIndex = i;
    }
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
    foreach (QString listItem, listAvailable)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->listAvailableList->addItem(item);
    }
    ui->listAvailableList->sortItems();

    ui->listCurrentList->clear();
    foreach (QString listItem, listCurrent)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->listCurrentList->addItem(item);
    }

    int listSortIndex = 0;
    QString currentListSort = SETTINGS.value("List/sort","Filename").toString();

    ui->listSortBox->clear();
    for (int i = 0; i < sortOptions.length(); i++)
    {
        ui->listSortBox->insertItem(i, getTranslation(sortOptions.at(i)), sortOptions.at(i));
        if (currentListSort == sortOptions.at(i))
            listSortIndex = i;
    }
    if (listSortIndex >= 0) ui->listSortBox->setCurrentIndex(listSortIndex);
}


void SettingsDialog::removeColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = currentList->currentRow();

    if (row >= 0) {
        availableList->addItem(currentList->takeItem(row));
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


void SettingsDialog::toggleAccessory(int index, int i)
{
    int ctrl;

    if (i >= 0 && i <= 3)
        ctrl = i;
    else
        ctrl = ui->controllersTabWidget->currentIndex();

    QList<QWidget*> memPakEnable;
    memPakEnable << ctrlMemPak.at(ctrl)
                 << ctrlMemPakButton.at(ctrl)
                 << ctrlMemPakLabel.at(ctrl);

    QList<QWidget*> tPakEnable;
    tPakEnable << ctrlTPakROM.at(ctrl)
               << ctrlTPakROMButton.at(ctrl)
               << ctrlTPakROMLabel.at(ctrl)
               << ctrlTPakSave.at(ctrl)
               << ctrlTPakSaveButton.at(ctrl)
               << ctrlTPakSaveLabel.at(ctrl);

    QList<QWidget*> allEnable;
    allEnable << memPakEnable << tPakEnable;

    foreach (QWidget *next, allEnable)
        next->setEnabled(false);

    if (index == 2) //Controller Pak
        foreach (QWidget *next, memPakEnable)
            next->setEnabled(true);
    else if (index == 3) //Transfer Pak
        foreach (QWidget *next, tPakEnable)
            next->setEnabled(true);
}


void SettingsDialog::toggleController(bool active, int i)
{
    int ctrl;

    if (i >= 0 && i <= 3)
        ctrl = i;
    else
        ctrl = ui->controllersTabWidget->currentIndex();

    QList<QWidget*> cEnable;

    if (active)
        cEnable << ctrlAccessory.at(ctrl);
    else
        cEnable << ctrlAccessory.at(ctrl)
                << ctrlAccessoryLabel.at(ctrl)
                << ctrlMemPak.at(ctrl)
                << ctrlMemPakButton.at(ctrl)
                << ctrlMemPakLabel.at(ctrl)
                << ctrlTPakROM.at(ctrl)
                << ctrlTPakROMButton.at(ctrl)
                << ctrlTPakROMLabel.at(ctrl)
                << ctrlTPakSave.at(ctrl)
                << ctrlTPakSaveButton.at(ctrl)
                << ctrlTPakSaveLabel.at(ctrl);

    foreach (QWidget *next, cEnable)
        next->setEnabled(active);

    if (active)
        toggleAccessory(ctrlAccessory.at(ctrl)->currentIndex(), ctrl);
}


void SettingsDialog::toggleDownload(bool active)
{
    foreach (QWidget *next, downloadEnable)
        next->setEnabled(active);

    if (active)
        toggleListCover(ui->listCoverOption->isChecked());
}


void SettingsDialog::toggleGridColumn(bool active)
{
    if (active)
        ui->columnCountBox->setEnabled(false);
    else
        ui->columnCountBox->setEnabled(true);
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


void SettingsDialog::updateLanguageInfo()
{
    ui->languageInfoLabel->setHidden(false);

    const char *sourceText = "<b>Note:</b> Language changes will not take place until application restart.";

    QTranslator translator;
    QString language = ui->languageBox->itemData(ui->languageBox->currentIndex()).toString().toLower();
    QString resource = ":/locale/"+AppNameLower+"_"+language+".qm";
    if (QFileInfo(resource).exists()) {
        translator.load(resource);
        ui->languageInfoLabel->setText(translator.translate("SettingsDialog", sourceText));
    } else
        ui->languageInfoLabel->setText(sourceText);
}
