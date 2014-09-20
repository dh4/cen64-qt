QT       += core network xml

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
    src/aboutdialog.cpp \
    src/cen64qt.cpp \
    src/settingsdialog.cpp \
    src/treewidgetitem.cpp \
    src/clickablewidget.cpp

HEADERS += src/global.h \
    src/aboutdialog.h \
    src/cen64qt.h \
    src/settingsdialog.h \
    src/treewidgetitem.h \
    src/clickablewidget.h

RESOURCES += resources/cen64qt.qrc

FORMS += src/settingsdialog.ui
