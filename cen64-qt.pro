QT       += core

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


SOURCES += src/main.cpp \
    src/aboutdialog.cpp \
    src/cen64qt.cpp \
    src/settingsdialog.cpp \
    src/treewidgetitem.cpp

HEADERS  += \
    src/aboutdialog.h \
    src/cen64qt.h \
    src/global.h \
    src/settingsdialog.h \
    src/treewidgetitem.h

RESOURCES += \
    resources/cen64qt.qrc
