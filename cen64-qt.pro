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
macx:ICON = macosx/cen64.icns
win32:RC_FILE = windows/icon.rc


SOURCES += src/main.cpp \
    src/common.cpp \
    src/mainwindow.cpp \
    src/aboutdialog.cpp \
    src/settingsdialog.cpp \
    src/treewidgetitem.cpp \
    src/clickablewidget.cpp \
    src/v64converter.cpp \
    src/downloaddialog.cpp \
    src/logdialog.cpp \
    src/emulatorhandler.cpp

HEADERS += src/global.h \
    src/common.h \
    src/mainwindow.h \
    src/aboutdialog.h \
    src/settingsdialog.h \
    src/treewidgetitem.h \
    src/clickablewidget.h \
    src/v64converter.h \
    src/downloaddialog.h \
    src/logdialog.h \
    src/emulatorhandler.h

RESOURCES += resources/cen64qt.qrc

FORMS += src/settingsdialog.ui

win32|macx {
    CONFIG += staticlib
    DEFINES += QUAZIP_STATIC

    #Download quazip source and copy the quazip directory to project
    SOURCES += quazip/*.cpp
    SOURCES += quazip/*.c
    HEADERS += quazip/*.h
} else {
    LIBS += -lquazip
}
