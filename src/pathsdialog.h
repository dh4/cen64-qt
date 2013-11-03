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

#ifndef PATHSDIALOG_H
#define PATHSDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

#include "global.h"

class PathsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PathsDialog(QWidget *parent = 0);

private:
    QCheckBox *saveOption;
    QDialogButtonBox *buttonBox;
    QGridLayout *layout;
    QGridLayout *pathsLayout;
    QGridLayout *savesLayout;
    QGroupBox *paths;
    QGroupBox *saves;
    QLabel *cen64PathLabel;
    QLabel *eepromPathLabel;
    QLabel *pifPathLabel;
    QLabel *romPathLabel;
    QLabel *savesPathLabel;
    QLabel *sramPathLabel;
    QLineEdit *cen64Path;
    QLineEdit *eepromPath;
    QLineEdit *pifPath;
    QLineEdit *romPath;
    QLineEdit *savesPath;
    QLineEdit *sramPath;
    QPushButton *cen64Button;
    QPushButton *eepromButton;
    QPushButton *pifButton;
    QPushButton *romButton;
    QPushButton *savesButton;
    QPushButton *sramButton;

private slots:
    void browseCen64();
    void browseEEPROM();
    void browsePIF();
    void browseROM();
    void browseSaves();
    void browseSRAM();
    void editSettings();
    void toggleSaves(bool active);
};

#endif // PATHSDIALOG_H
