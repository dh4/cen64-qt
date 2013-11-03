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

#include "pathsdialog.h"


PathsDialog::PathsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Paths"));
    setMinimumWidth(500);

    layout = new QGridLayout(this);

    paths = new QGroupBox(tr("CEN64 Files"), this);
    pathsLayout = new QGridLayout(paths);

    cen64PathLabel = new QLabel(tr("Path to CEN64 executable:"), this);
    pifPathLabel = new QLabel(tr("Path to pifrom/pifdata file:"), this);
    romPathLabel = new QLabel(tr("Path to ROM directory:"), this);

    cen64Path = new QLineEdit(SETTINGS.value("cen64", "").toString(), this);
    pifPath = new QLineEdit(SETTINGS.value("pifrom", "").toString(), this);
    romPath = new QLineEdit(SETTINGS.value("roms", "").toString(), this);

    cen64Button = new QPushButton(tr("Browse"), this);
    pifButton = new QPushButton(tr("Browse"), this);
    romButton = new QPushButton(tr("Browse"), this);

    pathsLayout->addWidget(cen64PathLabel, 0, 0);
    pathsLayout->addWidget(pifPathLabel, 1, 0);
    pathsLayout->addWidget(romPathLabel, 2, 0);

    pathsLayout->addWidget(cen64Path, 0, 1);
    pathsLayout->addWidget(pifPath, 1, 1);
    pathsLayout->addWidget(romPath, 2, 1);

    pathsLayout->addWidget(cen64Button, 0, 2);
    pathsLayout->addWidget(pifButton, 1, 2);
    pathsLayout->addWidget(romButton, 2, 2);

    paths->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    paths->setLayout(pathsLayout);


    saves = new QGroupBox(tr("Save Files"), this);
    savesLayout = new QGridLayout(saves);

    savesPathLabel = new QLabel(tr("Save Directory:"), this);
    eepromPathLabel = new QLabel(tr("Path to EEPROM:"), this);
    sramPathLabel = new QLabel(tr("Path to SRAM:"), this);

    savesPath = new QLineEdit(SETTINGS.value("saves", "").toString(), this);
    eepromPath = new QLineEdit(SETTINGS.value("eeprom", "").toString(), this);
    sramPath = new QLineEdit(SETTINGS.value("sram", "").toString(), this);

    savesButton = new QPushButton(tr("Browse"), this);
    eepromButton = new QPushButton(tr("Browse"), this);
    sramButton = new QPushButton(tr("Browse"), this);

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

    if (SETTINGS.value("individualsave", "").toString() == "true") {
        toggleSaves(true);
        saveOption->setChecked(true);
    } else {
        toggleSaves(false);
    }

    saves->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    saves->setLayout(savesLayout);


    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    layout->addWidget(paths, 0, 0, 1, 2);
    layout->addWidget(saves, 1, 0, 1, 2);
    layout->addWidget(buttonBox, 3, 1);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    setLayout(layout);

    connect(cen64Button, SIGNAL(clicked()), this, SLOT(browseCen64()));
    connect(pifButton, SIGNAL(clicked()), this, SLOT(browsePIF()));
    connect(romButton, SIGNAL(clicked()), this, SLOT(browseROM()));
    connect(savesButton, SIGNAL(clicked()), this, SLOT(browseSaves()));
    connect(eepromButton, SIGNAL(clicked()), this, SLOT(browseEEPROM()));
    connect(sramButton, SIGNAL(clicked()), this, SLOT(browseSRAM()));
    connect(saveOption, SIGNAL(toggled(bool)), this, SLOT(toggleSaves(bool)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void PathsDialog::browseCen64()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "") {
        cen64Path->setText(path);
    }
}

void PathsDialog::browseEEPROM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "") {
        eepromPath->setText(path);
    }
}

void PathsDialog::browsePIF()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "") {
        pifPath->setText(path);
    }
}

void PathsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "") {
        romPath->setText(path);
    }
}

void PathsDialog::browseSaves()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "") {
        savesPath->setText(path);
    }
}

void PathsDialog::browseSRAM()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "") {
        sramPath->setText(path);
    }
}

void PathsDialog::editSettings()
{
    SETTINGS.setValue("cen64", cen64Path->text());
    SETTINGS.setValue("pifrom", pifPath->text());
    SETTINGS.setValue("roms", romPath->text());

    SETTINGS.setValue("saves", savesPath->text());
    SETTINGS.setValue("eeprom", eepromPath->text());
    SETTINGS.setValue("sram", sramPath->text());

    if (saveOption->isChecked()) {
        SETTINGS.setValue("individualsave", true);
    } else {
        SETTINGS.setValue("individualsave", "");
    }

    close();
}

void PathsDialog::toggleSaves(bool active)
{
    if (active) {
        savesPathLabel->setEnabled(false);
        eepromPathLabel->setEnabled(true);
        sramPathLabel->setEnabled(true);
        savesPath->setEnabled(false);
        eepromPath->setEnabled(true);
        sramPath->setEnabled(true);
        savesButton->setEnabled(false);
        eepromButton->setEnabled(true);
        sramButton->setEnabled(true);
    } else {
        savesPathLabel->setEnabled(true);
        eepromPathLabel->setEnabled(false);
        sramPathLabel->setEnabled(false);
        savesPath->setEnabled(true);
        eepromPath->setEnabled(false);
        sramPath->setEnabled(false);
        savesButton->setEnabled(true);
        eepromButton->setEnabled(false);
        sramButton->setEnabled(false);
    }
}
