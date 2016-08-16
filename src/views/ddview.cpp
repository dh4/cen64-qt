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

#include "ddview.h"

#include "../common.h"
#include "../global.h"

#include "widgets/treewidgetitem.h"

#include <QFileInfo>
#include <QHeaderView>


DDView::DDView(QWidget *parent) : QTreeWidget(parent)
{ 
    this->parent = parent;

    setWordWrap(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");
    setHeaderLabels(QStringList() << "" << "" << "" << "" << "" << "64DD ROM");
    header()->setHidden(true);
    setColumnHidden(0, true); //Hidden filename for launching emulator
    setColumnHidden(1, true); //Hidden directory of ROM location
    setColumnHidden(2, true); //Hidden goodname for searching
    setColumnHidden(3, true); //Hidden md5 for cache info
    setColumnHidden(4, true); //Hidden column for zip file
    setHidden(true);
}


void DDView::addNoDiskRow()
{
    fileItem = new TreeWidgetItem(this);
    fileItem->setText(5, tr("No Disk"));
    fileItem->setForeground(5, QBrush(Qt::gray));
    addTopLevelItem(fileItem);
}


void DDView::addTo64DDView(Rom *currentRom)
{
    fileItem = new TreeWidgetItem(this);

    fileItem->setText(0, currentRom->fileName); //Filename for launching ROM
    fileItem->setText(1, currentRom->directory); //Directory ROM is located in
    fileItem->setText(2, ""); //GoodName or Internal Name for searching (currently blank)
    fileItem->setText(3, currentRom->romMD5.toLower()); //MD5 for cache info
    fileItem->setText(4, currentRom->zipFile); //Zip file

    fileItem->setText(5, QFileInfo(currentRom->fileName).completeBaseName()); //Visible filename

    addTopLevelItem(fileItem);
}


QString DDView::getCurrentRomInfo(QString infoName)
{
    int index = getTableDataIndexFromName(infoName);
    return QVariant(currentItem()->data(index, 0)).toString();
}


bool DDView::hasSelectedRom()
{
    return currentItem() != NULL;
}
