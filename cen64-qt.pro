QT       += core

lessThan(QT_MAJOR_VERSION, 5) {
    QT   += gui
} else {
    QT   += widgets
}

TARGET = cen64-qt
TEMPLATE = app


SOURCES += src/main.cpp \
    src/aboutdialog.cpp \
    src/cen64qt.cpp \
    src/pathsdialog.cpp

HEADERS  += \
    src/aboutdialog.h \
    src/pathsdialog.h \
    src/cen64qt.h \
    src/global.h

RESOURCES += \
    resources/cen64qt.qrc
