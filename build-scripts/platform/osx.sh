#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')


case "$1" in

    'setup_qt')
        mkdir $WORKING_DIR/../osx
        cd $WORKING_DIR/../osx

        if [[ $BUILD_OSX_QT ]]; then
            # Build Qt
            git clone --branch v5.5.0 --depth 1 https://code.qt.io/qt/qtbase.git
            cd qtbase
            ./configure -release -static -qt-sql-sqlite -opensource -confirm-license
            make sub-src
        else
            # Fetch pre-build Qt
            wget https://www.dropbox.com/s/mfw6qwfir62fy9p/macosx-qt5.tar.gz
            tar -xvzf macosx-qt5.tar.gz >& /dev/null
        fi

        cd $WORKING_DIR/cen64-qt
    ;;

    'get_quazip')
        wget http://downloads.sourceforge.net/quazip/quazip-0.7.1.tar.gz
        tar -xvzf quazip-0.7.1.tar.gz >& /dev/null
        mv quazip-0.7.1/quazip .
    ;;

    'build')
        ./build-scripts/revision.sh
        $WORKING_DIR/../osx/qtbase/bin/qmake -config release LIBS+="-dead_strip"
        make
    ;;

    'package')
        mkdir build

        hdiutil create -megabytes 20 -fs HFS+ -volname CEN64-Qt cen64-qt_osx_$VERSION
        hdiutil attach cen64-qt_osx_$VERSION.dmg

        cp -r CEN64-Qt.app /Volumes/CEN64-Qt/CEN64-Qt.app
        cp resources/README.txt /Volumes/CEN64-Qt/README.txt

        hdiutil detach /Volumes/CEN64-Qt
        hdiutil convert -format UDZO -o cen64-qt_osx_$VERSION.dmg \
                        -ov cen64-qt_osx_$VERSION.dmg

        mv cen64-qt_osx_$VERSION.dmg build/
    ;;

esac
