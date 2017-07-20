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

#include "v64converter.h"

#include "../global.h"

#include <QFileDialog>
#include <QMessageBox>


V64Converter::V64Converter(QString romPath, QWidget *parent) : QObject(parent)
{
    QDir romDir(romPath);

    QString v64File = QFileDialog::getOpenFileName(parent, tr("Open v64/n64 File"), romPath,
                                                   tr("V64/n64 ROMs") + " (*.v64 *.n64);;" +
                                                   tr("All Files") + " (*)");

    if (v64File != "") {
        QString defaultFileName = QFileInfo(v64File).completeBaseName() + ".z64";
        QString defaultFile = romDir.absoluteFilePath(defaultFileName);
        QString saveFile = QFileDialog::getSaveFileName(parent, tr("Save z64 File"), defaultFile,
                                                        tr("Z64 ROMs") + " (*.z64);;" +
                                                        tr("All Files") + " (*)");

        if (saveFile != "")
            runConverter(v64File, saveFile, parent);
    }
}


void V64Converter::runConverter(QString v64File, QString saveFile, QWidget *parent)
{
    QFile v64(v64File);
    v64.open(QIODevice::ReadOnly);

    QString v64Check(v64.read(4).toHex()), message;
    if (v64Check == "80371240") {
        message = "\"" + QFileInfo(v64).fileName() + "\" " + tr("already in z64 format!");
        QMessageBox::warning(parent, tr("<AppName> Converter").replace("<AppName>",AppName), message);
    } else if (v64Check == "37804012" || v64Check == "40123780") {
        v64.seek(0);

        QFile z64(saveFile);
        z64.open(QIODevice::WriteOnly);

        QByteArray data;
        QByteArray flipped;

        while (!v64.atEnd())
        {
            data = v64.read(1024);

            if (v64Check == "37804012") { // byte-swapped
                for (int i = 0; i < data.size(); i+=2)
                {
                    //Check to see if only one byte remaining (though byte count should always be even)
                    if (i + 1 == data.size())
                        flipped.append(data[i]);
                    else {
                        flipped.append(data[i + 1]);
                        flipped.append(data[i]);
                    }
                }
            } else { // little-endian
                for (int i = 0; i < data.size(); i+=4)
                {
                    //Check to see if less than 4 bytes remaining (though byte count should always be even)
                    if (i + 3 > data.size()) {
                        for (int n = data.size(); n >= i; n--)
                            flipped.append(data[n]);
                    } else {
                        for (int n = i + 3; n >= i; n--)
                            flipped.append(data[n]);
                    }
                }
            }

            z64.write(flipped);

            flipped.truncate(0);
        }

        z64.close();
        QMessageBox::information(parent, tr("<AppName> Converter").replace("<AppName>",AppName),
                                 tr("Conversion complete!"));
    } else {
        message = "\"" + QFileInfo(v64).fileName() + "\" " + tr("is not a valid .v64 or .n64 file!");
        QMessageBox::warning(parent, tr("<AppName> Converter").replace("<AppName>",AppName), message);
    }

    v64.close();
}
