QT       += core widgets network xml sql

macx {
    TARGET = CEN64-Qt
} else {
    TARGET = cen64-qt
}

TEMPLATE = app
macx:ICON = dist/macosx/cen64.icns
win32:RC_FILE = dist/windows/icon.rc


SOURCES += src/main.cpp \
    src/common.cpp \
    src/mainwindow.cpp \
    src/dialogs/aboutdialog.cpp \
    src/dialogs/downloaddialog.cpp \
    src/dialogs/logdialog.cpp \
    src/dialogs/settingsdialog.cpp \
    src/dialogs/v64converter.cpp \
    src/emulation/emulatorhandler.cpp \
    src/roms/romcollection.cpp \
    src/roms/thegamesdbscraper.cpp \
    src/views/gridview.cpp \
    src/views/listview.cpp \
    src/views/tableview.cpp \
    src/views/ddview.cpp \
    src/views/widgets/clickablewidget.cpp \
    src/views/widgets/treewidgetitem.cpp

HEADERS += src/global.h \
    src/common.h \
    src/mainwindow.h \
    src/dialogs/aboutdialog.h \
    src/dialogs/downloaddialog.h \
    src/dialogs/logdialog.h \
    src/dialogs/settingsdialog.h \
    src/dialogs/v64converter.h \
    src/emulation/emulatorhandler.h \
    src/roms/romcollection.h \
    src/roms/thegamesdbscraper.h \
    src/views/gridview.h \
    src/views/listview.h \
    src/views/tableview.h \
    src/views/ddview.h \
    src/views/widgets/clickablewidget.h \
    src/views/widgets/treewidgetitem.h

RESOURCES += resources/cen64qt.qrc

FORMS += src/dialogs/settingsdialog.ui

TRANSLATIONS += resources/locale/cen64-qt_fr.ts \
    resources/locale/cen64-qt_ru.ts

win32|macx|linux_quazip_static {
    QT += core5compat
    CONFIG += staticlib
    DEFINES += QUAZIP_STATIC
    LIBS += -lz

    #Download quazip source and copy the quazip directory to project as quazip5
    SOURCES += quazip/*.cpp
    SOURCES += quazip/*.c
    HEADERS += quazip/*.h
} else {
     LIBS += -lquazip1-qt6
}
