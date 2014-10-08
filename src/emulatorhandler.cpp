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

#include "emulatorhandler.h"


EmulatorHandler::EmulatorHandler(QObject *parent) : QObject(parent)
{
    lastOutput = "";
}

void EmulatorHandler::checkStatus(int status)
{
    if (status > 0)
        QMessageBox::warning(0, tr("Warning"),
            tr("CEN64 quit unexpectedly. Check to make sure you are using a valid ROM."));
    else
        updateStatus("Emulation stopped", 3000);
}


void EmulatorHandler::cleanTemp()
{
    QFile::remove(QDir::tempPath() + "/cen64-qt/temp.z64");
}


void EmulatorHandler::emitFinished()
{
    emit finished();
}


void EmulatorHandler::readOutput()
{
    QString output = emulatorProc->readAllStandardOutput();
    QStringList outputList = output.split("\n");

    int lastIndex = outputList.lastIndexOf(QRegExp("^.*VI/s.*MHz$"));

    if (lastIndex >= 0)
        updateStatus(outputList[lastIndex]);

    lastOutput.append(output);
}


void EmulatorHandler::startEmulator(QDir romDir, QString romFileName, QString zipFileName)
{
    QString completeRomPath;
    bool zip = false;

    if (zipFileName != "") { //If zipped file, extract and write to temp location for loading
        zip = true;

        QString zipFile = romDir.absoluteFilePath(zipFileName);
        QByteArray *romData = getZippedRom(romFileName, zipFile);

        QString tempDir = QDir::tempPath() + "/cen64-qt";
        QDir().mkdir(tempDir);
        completeRomPath = tempDir + "/temp.z64";

        QFile tempRom(completeRomPath);
        tempRom.open(QIODevice::WriteOnly);
        tempRom.write(*romData);
        tempRom.close();

        delete romData;
    } else
        completeRomPath = romDir.absoluteFilePath(romFileName);

    QString cen64Path = SETTINGS.value("Paths/cen64", "").toString();
    QString pifPath = SETTINGS.value("Paths/pifrom", "").toString();
    QString input = SETTINGS.value("input", "").toString();

    QFile cen64File(cen64Path);
    QFile pifFile(pifPath);
    QFile romFile(completeRomPath);


    //Sanity checks
    if (!cen64File.exists() || QFileInfo(cen64File).isDir() || !QFileInfo(cen64File).isExecutable()) {
        QMessageBox::warning(0, tr("Warning"), tr("CEN64 executable not found."));
        if (zip) cleanTemp();
        return;
    }

    if (!pifFile.exists() || QFileInfo(pifFile).isDir()) {
        QMessageBox::warning(0, tr("Warning"), tr("PIFdata file not found."));
        if (zip) cleanTemp();
        return;
    }

    if (!romFile.exists() || QFileInfo(romFile).isDir()) {
        QMessageBox::warning(0, tr("Warning"), tr("ROM file not found."));
        if (zip) cleanTemp();
        return;
    }

    romFile.open(QIODevice::ReadOnly);
    QByteArray romCheck = romFile.read(4);
    romFile.close();

    if (romCheck.toHex() != "80371240") {
        QMessageBox::warning(0, tr("Warning"), tr("Not a valid Z64 File."));
        if (zip) cleanTemp();
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
            QDir savesDir(savesPath);

            if (savesDir.exists()) {
                romFile.open(QIODevice::ReadOnly);
                QByteArray *romData = new QByteArray(romFile.readAll());
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

    emulatorProc = new QProcess(this);
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(emitFinished()));
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    if (zip)
        connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(cleanTemp()));


    if (SETTINGS.value("Other/consoleoutput", "").toString() == "true")
        emulatorProc->setProcessChannelMode(QProcess::ForwardedChannels);
    else {
        connect(emulatorProc, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
        emulatorProc->setProcessChannelMode(QProcess::MergedChannels);
    }

    //clear log
    lastOutput = "";

    emulatorProc->start(cen64Path, args);

    updateStatus("Emulation started", 3000);
    emit started();
}


void EmulatorHandler::stopEmulator()
{
    emulatorProc->terminate();
}


void EmulatorHandler::updateStatus(QString message, int timeout)
{
    emit statusUpdate(message, timeout);
}
