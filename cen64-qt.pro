QT       += core network xml sql

lessThan(QT_MAJOR_VERSION, 5) {
    QT   += gui
} else {
    QT   += widgets
}

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
    src/aboutdialog.cpp \
    src/settingsdialog.cpp \
    src/v64converter.cpp \
    src/downloaddialog.cpp \
    src/logdialog.cpp \
    src/emulatorhandler.cpp \
    src/romcollection.cpp \
    src/thegamesdbscraper.cpp \
    src/views/clickablewidget.cpp \
    src/views/gridview.cpp \
    src/views/listview.cpp \
    src/views/tableview.cpp \
    src/views/ddview.cpp \
    src/views/treewidgetitem.cpp

HEADERS += src/global.h \
    src/common.h \
    src/mainwindow.h \
    src/aboutdialog.h \
    src/settingsdialog.h \
    src/v64converter.h \
    src/downloaddialog.h \
    src/logdialog.h \
    src/emulatorhandler.h \
    src/romcollection.h \
    src/thegamesdbscraper.h \
    src/views/clickablewidget.h \
    src/views/gridview.h \
    src/views/listview.h \
    src/views/tableview.h \
    src/views/ddview.h \
    src/views/treewidgetitem.h

RESOURCES += resources/cen64qt.qrc

FORMS += src/settingsdialog.ui

TRANSLATIONS += resources/locale/cen64-qt_fr.ts

win32|macx|linux_quazip_static {
    CONFIG += staticlib
    DEFINES += QUAZIP_STATIC

    #Download quazip source and copy the quazip directory to project
    SOURCES += quazip/*.cpp
    SOURCES += quazip/*.c
    HEADERS += quazip/*.h
} else {
    lessThan(QT_MAJOR_VERSION, 5) {
        LIBS += -lquazip
    } else {
        # Debian distributions use a different library name for Qt5 quazip
        system("which dpkg > /dev/null 2>&1") {
            system("dpkg -l | grep libquazip-qt5-dev | grep ^ii > /dev/null") {
                LIBS += -lquazip-qt5
            } else {
                LIBS += -lquazip5
            }
        } else {
            LIBS += -lquazip5
        }
    }
}
