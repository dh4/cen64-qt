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
        exitDialog.setText(tr("CEN64 quit unexpectedly. Check the log for more information."));
        exitDialog.setIcon(QMessageBox::Warning);
        exitDialog.addButton(QMessageBox::Ok);
        exitDialog.addButton("View Log...", QMessageBox::HelpRole);

        int ret = exitDialog.exec();
        if (ret == 0) emit showLog();
    }

    updateStatus("Emulation stopped", 3000);
}


void EmulatorHandler::cleanTemp()
{
    QFile::remove(QDir::tempPath() + "/cen64-qt/temp.bin");
    QFile::remove(QDir::tempPath() + "/cen64-qt/64dd-temp.bin");
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

            QString tempDir = QDir::tempPath() + "/cen64-qt";
            QDir().mkdir(tempDir);

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

    QString cen64Path = SETTINGS.value("Paths/cen64", "").toString();
    QString pifPath = SETTINGS.value("Paths/pifrom", "").toString();
    QString ddIPLPath = SETTINGS.value("Paths/ddiplrom", "").toString();
    QString input = SETTINGS.value("input", "").toString();

    bool ddMode = false;
    if (SETTINGS.value("Emulation/64dd", "").toString() == "true")
        ddMode = true;

    QFile cen64File(cen64Path);
    QFile pifFile(pifPath);
    QFile ddIPL(ddIPLPath);
    QFile romFile(completeRomPath);
    QFile ddFile(complete64DDPath);


    //Sanity checks
    if (!cen64File.exists() || QFileInfo(cen64File).isDir() || !QFileInfo(cen64File).isExecutable()) {
        QMessageBox::warning(parent, tr("Warning"), tr("CEN64 executable not found."));
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
//    args << "-controller" << input;

//    if (SETTINGS.value("Saves/individualsave", "").toString() == "true") {
//        QString eepromPath = SETTINGS.value("Saves/eeprom", "").toString();
//        QString sramPath = SETTINGS.value("Saves/sram", "").toString();

//        if (eepromPath != "")
//            args << "-eeprom" << eepromPath;

//        if (sramPath != "")
//            args << "-sram" << sramPath;
//    } else {
//        QString savesPath = SETTINGS.value("Saves/directory", "").toString();
//        if (savesPath != "") {
//            QDir savesDir(savesPath);

//            if (savesDir.exists()) {
//                romFile.open(QIODevice::ReadOnly);
//                QByteArray *romData = new QByteArray(romFile.readAll());
//                romFile.close();

//                QString romMD5 = QString(QCryptographicHash::hash(*romData,
//                                                                  QCryptographicHash::Md5).toHex());

//                QString romBaseName = QFileInfo(romFile).completeBaseName();
//                QString eepromFileName = romBaseName + "." + romMD5 + ".eeprom";
//                QString sramFileName = romBaseName + "." + romMD5 + ".sram";
//                QString eepromPath = savesDir.absoluteFilePath(eepromFileName);
//                QString sramPath = savesDir.absoluteFilePath(sramFileName);

//                args << "-eeprom" << eepromPath << "-sram" << sramPath;

//                delete romData;
//            }
//        }
//    }

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
