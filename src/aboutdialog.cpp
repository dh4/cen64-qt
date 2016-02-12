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

#include "aboutdialog.h"
#include "common.h"
#include "global.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPlainTextEdit>


AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About CEN64-Qt"));
    setMinimumSize(600, 300);

    aboutLayout = new QGridLayout(this);

    icon = new QLabel(this);
    icon->setPixmap(QPixmap(":/images/cen64.png"));

    QFile licenseFile(":/other/LICENSE");
    licenseFile.open(QIODevice::ReadOnly);
    license = new QPlainTextEdit(licenseFile.readAll(), this);
    license->setReadOnly(true);
    licenseFile.close();

    QString description = "<b>CEN64-Qt</b><br />" + tr("Version") + " " + getVersion() + "<br /><br />";
    description += tr("A basic frontend for CEN64 using Qt.");
    QString copyright = "CEN64 Copyright (c) 2013, Tyler J. Stachecki";
    QString cen64 = "<a href=\"http://www.cen64.com/\">" + tr("CEN64 website") + "</a>";
    QString github = "<a href=\"https://github.com/dh4/cen64-qt\">" + tr("GitHub repository") + "</a>";

    descriptionLabel = new QLabel(description, this);
    copyrightLabel = new QLabel(copyright, this);
    cen64Link = new QLabel(cen64, this);
    githubLink = new QLabel(github, this);

    cen64Link->setOpenExternalLinks(true);
    githubLink->setOpenExternalLinks(true);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);

    aboutLayout->addWidget(icon, 0, 0, 4, 1);
    aboutLayout->addWidget(descriptionLabel, 0, 1);
    aboutLayout->addWidget(license, 1, 1);
    aboutLayout->addWidget(copyrightLabel, 2, 1);
    aboutLayout->addWidget(cen64Link, 3, 1);
    aboutLayout->addWidget(githubLink, 4, 1);
    aboutLayout->addWidget(buttonBox, 5, 1);
    aboutLayout->setColumnStretch(1, 1);
    aboutLayout->setRowStretch(1, 1);
    aboutLayout->setColumnMinimumWidth(0, 150);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));

    setLayout(aboutLayout);
}
