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

#include "emulatorhandler.h"

#include "../global.h"
#include "../common.h"

#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QCryptographicHash>

#if QT_VERSION >= 0x050000
#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>
#else
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#endif


EmulatorHandler::EmulatorHandler(QWidget *parent) : QObject(parent)
{
    this->parent = parent;

    lastOutput = "";
}

void EmulatorHandler::checkStatus(int status)
{
    if (status > 0) {
        QMessageBox exitDialog(parent);
        exitDialog.setWindowTitle(tr("Warning"));
        exitDialog.setText(tr("<ParentName> quit unexpectedly. Check the log for more information.")
                           .replace("<ParentName>",ParentName));
        exitDialog.setIcon(QMessageBox::Warning);
        exitDialog.addButton(QMessageBox::Ok);
        exitDialog.addButton(tr("View Log..."), QMessageBox::HelpRole);

        int ret = exitDialog.exec();
        if (ret == 0) emit showLog();
    }

    updateStatus(tr("Emulation stopped"), 3000);
}


void EmulatorHandler::cleanTemp()
{
    QFile::remove(QDir::tempPath() + "/" + AppNameLower + "/" + qgetenv("USER") + "/temp.bin");
    QFile::remove(QDir::tempPath() + "/" + AppNameLower + "/" + qgetenv("USER") + "/64dd-temp.bin");
}


void EmulatorHandler::emitFinished()
{
    emit finished();
}


QStringList EmulatorHandler::parseArgString(QString argString)
{
    QStringList result;
    QString arg;
    bool inQuote = false;
    bool inApos = false;

    for (int i = 0; i < argString.size(); i++)
    {
        // Check if inside of a quote
        if (argString.at(i) == QLatin1Char('"')) {
            inQuote = !inQuote;

            // Only continue if outside of both quotes and apostrophes
            if (arg.isEmpty() || (!inQuote && !inApos)) continue;
        }

        // Same check for apostrophes
        if (argString.at(i) == QLatin1Char('\'')) {
            inApos = !inApos;
            if (arg.isEmpty() || (!inQuote && !inApos)) continue;
        }

        if (!inQuote && !inApos && argString.at(i).isSpace()) {
            if (!arg.isEmpty()) {
                result += arg;
                arg.clear();
            }
        } else
            arg += argString.at(i);
    }

    if (!arg.isEmpty())
        result += arg;

    return result;
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


void EmulatorHandler::startEmulator(QDir romDir, QString romFileName, QString zipFileName,
                                    QDir ddDir, QString ddFileName, QString ddZipName)
{
    QString completeRomPath = "", complete64DDPath = "";
    bool zip = false, ddZip = false;

    //If zipped file, extract and write to temp location for loading
    QStringList zippedFiles;
    zippedFiles << zipFileName << ddZipName;
    bool ddZipCheck = false;

    foreach (QString zippedFile, zippedFiles)
    {
        if (zippedFile != "") {
            QString fileInZip, tempName, romPath, zipFile;

            if (!ddZipCheck) {
                zip = true;
                tempName = "/temp.bin";
                fileInZip = romFileName;
                zipFile = romDir.absoluteFilePath(zippedFile);
            } else {
                ddZip = true;
                tempName = "/64dd-temp.bin";
                fileInZip = ddFileName;
                zipFile = ddDir.absoluteFilePath(zippedFile);
            }

            QByteArray *romData = getZippedRom(fileInZip, zipFile);

            QString tempDir = QDir::tempPath() + "/" + AppNameLower + "/" + qgetenv("USER");
            QDir().mkpath(tempDir);

            romPath = tempDir + tempName;
            QFile tempRom(romPath);

            tempRom.open(QIODevice::WriteOnly);
            tempRom.write(*romData);
            tempRom.close();

            delete romData;

            if (!ddZipCheck)
                completeRomPath = romPath;
            else
                complete64DDPath = romPath;
        }

        ddZipCheck = true;
    }

    if (zipFileName == "" && romFileName != "")
        completeRomPath = romDir.absoluteFilePath(romFileName);
    if (ddZipName == "" && ddFileName != "")
        complete64DDPath = ddDir.absoluteFilePath(ddFileName);

    QString emulatorPath = SETTINGS.value("Paths/cen64", "").toString();
    QString pifPath = SETTINGS.value("Paths/pifrom", "").toString();
    QString ddIPLPath = SETTINGS.value("Paths/ddiplrom", "").toString();

    bool ddMode = false;
    if (SETTINGS.value("Emulation/64dd", "").toString() == "true")
        ddMode = true;

    QFile emulatorFile(emulatorPath);
    QFile pifFile(pifPath);
    QFile ddIPL(ddIPLPath);
    QFile romFile(completeRomPath);
    QFile ddFile(complete64DDPath);


    //Sanity checks
    if (!emulatorFile.exists() || QFileInfo(emulatorFile).isDir() || !QFileInfo(emulatorFile).isExecutable()) {
        QMessageBox::warning(parent, tr("Warning"),
                             tr("<ParentName> executable not found.").replace("<ParentName>",ParentName));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (!pifFile.exists() || QFileInfo(pifFile).isDir()) {
        QMessageBox::warning(parent, tr("Warning"), tr("PIF IPL file not found."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (ddIPLPath != "" && (!ddIPL.exists() || QFileInfo(ddIPL).isDir())) {
        QMessageBox::warning(parent, tr("Warning"), tr("64DD IPL file not found."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (completeRomPath != "" && (!romFile.exists() || QFileInfo(romFile).isDir())) {
        QMessageBox::warning(parent, tr("Warning"), tr("ROM file not found."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (completeRomPath == "" && complete64DDPath != ""
            && (!ddFile.exists() || QFileInfo(ddFile).isDir())) {
        QMessageBox::warning(parent, tr("Warning"), tr("64DD ROM file not found."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (completeRomPath == "" && complete64DDPath == "") {
        QMessageBox::warning(parent, tr("Warning"), tr("No ROM selected."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    if (completeRomPath != "") {
        romFile.open(QIODevice::ReadOnly);
        QByteArray romCheck = romFile.read(4);
        romFile.close();

        if (romCheck.toHex() != "80371240") {
            if (romCheck.toHex() == "e848d316") { // 64DD file instead
                if (complete64DDPath == "" && ddMode) {
                    complete64DDPath = completeRomPath;
                    ddFile.setFileName(complete64DDPath);
                    completeRomPath = "";
                } else {
                    QMessageBox::warning(parent, tr("Warning"), tr("64DD not enabled."));
                    if (zip || ddZip) cleanTemp();
                    return;
                }
            } else {
                QMessageBox::warning(parent, tr("Warning"), tr("Not a valid Z64 File."));
                if (zip || ddZip) cleanTemp();
                return;
            }
        }
    }

    if (complete64DDPath != "" && ddMode) {
        ddFile.open(QIODevice::ReadOnly);
        QByteArray romCheck = ddFile.read(4);
        ddFile.close();

        if (romCheck.toHex() != "e848d316") {
            QMessageBox::warning(parent, tr("Warning"), tr("Not a valid 64DD File."));
            if (zip || ddZip) cleanTemp();
            return;
        }
    }


    QStringList args;

    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
        QString eeprom4kPath = SETTINGS.value("Saves/eeprom4k", "").toString();
        QString eeprom16kPath = SETTINGS.value("Saves/eeprom16k", "").toString();
        QString sramPath = SETTINGS.value("Saves/sram", "").toString();
        QString flashPath = SETTINGS.value("Saves/flash", "").toString();

        if (eeprom4kPath != "")
            args << "-eep4k" << eeprom4kPath;
        if (eeprom16kPath != "")
            args << "-eep16k" << eeprom16kPath;
        if (sramPath != "")
            args << "-sram" << sramPath;
        if (flashPath != "")
            args << "-flash" << flashPath;
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
                QString eeprom4kFileName = romBaseName + "." + romMD5 + ".eep4k";
                QString eeprom16kFileName = romBaseName + "." + romMD5 + ".eep16k";
                QString sramFileName = romBaseName + "." + romMD5 + ".sram";
                QString flashFileName = romBaseName + "." + romMD5 + ".flash";
                QString eeprom4kPath = savesDir.absoluteFilePath(eeprom4kFileName);
                QString eeprom16kPath = savesDir.absoluteFilePath(eeprom16kFileName);
                QString sramPath = savesDir.absoluteFilePath(sramFileName);
                QString flashPath = savesDir.absoluteFilePath(flashFileName);

                // Check ROM catalog to determine save type
                QString catalogFile = SETTINGS.value("Paths/catalog", "").toString();
                if (QFileInfo(catalogFile).exists()) {
                    QSettings romCatalog(catalogFile, QSettings::IniFormat, parent);
                    QString saveType = romCatalog.value(romMD5.toUpper()+"/SaveType","").toString();

                    if (saveType == "Eeprom 4KB")
                        args << "-eep4k"  << eeprom4kPath;
                    else if (saveType == "Eeprom 16KB")
                        args << "-eep16k"  << eeprom16kPath;
                    else if (saveType == "SRAM")
                        args << "-sram"  << sramPath;
                    else if (saveType == "Flash RAM")
                        args << "-flash"  << flashPath;
                    else if (saveType == "Controller Pack");
                    else
                        args << "-eep4k"  << eeprom4kPath
                             << "-eep16k" << eeprom16kPath
                             << "-sram"   << sramPath
                             << "-flash"  << flashPath;
                } else {
                    args << "-eep4k"  << eeprom4kPath
                         << "-eep16k" << eeprom16kPath
                         << "-sram"   << sramPath
                         << "-flash"  << flashPath;
                }

                delete romData;
            }
        }
    }

    for (int i = 1; i <= 4; i++)
    {
        QString ctrl = "Controller"+QString::number(i);

        if (SETTINGS.value(ctrl+"/enabled", "").toString() == "true") {
            args << "-controller";
            QString options = "num="+QString::number(i);

            int accessory = SETTINGS.value(ctrl+"/accessory", 0).toInt();
            QString memPak = SETTINGS.value(ctrl+"/mempak", "").toString();
            QString tPakROM = SETTINGS.value(ctrl+"/tpakrom", "").toString();
            QString tPakSave = SETTINGS.value(ctrl+"/tpaksave", "").toString();

            if (accessory == 1) //Rumble Pak
                options += ",pak=rumble";
            else if (accessory == 2 && memPak != "") //Controller Pak
                options += ",mempak="+memPak;
            else if (accessory == 3 && tPakROM != "" && tPakSave != "") //Transfer Pak
                options += ",tpak_rom="+tPakROM+",tpak_save="+tPakSave;

            args << options;
        }
    }

    if (SETTINGS.value("Emulation/multithread", "").toString() == "true")
        args << "-multithread";
    if (SETTINGS.value("Emulation/noaudio", "").toString() == "true")
        args << "-noaudio";
    if (SETTINGS.value("Emulation/novideo", "").toString() == "true")
        args << "-novideo";

    if (ddIPLPath != "" && complete64DDPath != "" && ddMode)
        args << "-ddipl" << ddIPLPath << "-ddrom" << complete64DDPath;
    else if (completeRomPath == "") {
        QMessageBox::warning(parent, tr("Warning"), tr("No ROM selected or 64DD not enabled."));
        if (zip || ddZip) cleanTemp();
        return;
    }

    QString otherParameters = SETTINGS.value("Other/parameters", "").toString();
    if (otherParameters != "")
        args.append(parseArgString(otherParameters));

    args << pifPath;

    if (completeRomPath != "")
        args << completeRomPath;

    emulatorProc = new QProcess(this);
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(emitFinished()));
    connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    if (zip || ddZip)
        connect(emulatorProc, SIGNAL(finished(int)), this, SLOT(cleanTemp()));


    if (SETTINGS.value("Other/consoleoutput", "").toString() == "true")
        emulatorProc->setProcessChannelMode(QProcess::ForwardedChannels);
    else {
        connect(emulatorProc, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
        emulatorProc->setProcessChannelMode(QProcess::MergedChannels);
    }

    //clear log
    lastOutput = "";

    emulatorProc->start(emulatorPath, args);

    //Add command to log
    QString executable = emulatorPath;
    if (executable.contains(" "))
        executable = '"' + executable + '"';

    QString argString;

    foreach(QString arg, args)
    {
        if (arg.contains(" "))
            argString += " \"" + arg + "\"";
        else
            argString += " " + arg;
    }

    lastOutput.append(executable + argString + "\n\n");


    updateStatus(tr("Emulation started"), 3000);
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
