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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QSettings>


const QString AppName           = "CEN64-Qt";
const QString AppNameLower      = "cen64-qt";
const QString ParentName        = "CEN64";
const QString ParentNameLower   = "cen64";

const QString Description = QObject::tr("A customizable frontend for <ParentName> using Qt.").replace("<ParentName>",ParentName);
const QString Copyright = "CEN64 Copyright (c) 2013, Tyler J. Stachecki";
const QString Website = "<a href=\"http://www.cen64.com/\">"
        + QObject::tr("<ParentName> website").replace("<ParentName>",ParentName)
        + "</a>";
const QString GitHub = "<a href=\"https://github.com/dh4/cen64-qt\">"
        + QObject::tr("GitHub repository")
        + "</a>";

const QString TheGamesDBAPIKey = "750dbe45c2f1c63dd3a730b0ef0923ec448b543a908fbe3e2bccada4341df2f5";

#ifdef Q_OS_WIN
    #define SETTINGS    QSettings(AppNameLower+".ini", QSettings::IniFormat)
#else
    #define SETTINGS    QSettings(ParentNameLower, AppNameLower)
#endif

#endif // GLOBAL_H
