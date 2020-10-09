#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')

[[ -z $ARCH ]] && ARCH=".$(uname -m)"
[[ $ARCH == ".x86_64" ]] && ARCH=""


case "$1" in

    'setup_qt')
        mkdir "$WORKING_DIR/../osx"
        cd "$WORKING_DIR/../osx"

        if [[ $BUILD_OSX_QT ]]; then
            # Build Qt
            # Perform commands in /Users/travis/build/osx/ if building for travis
            git clone --branch v5.10.0 --depth 1 https://code.qt.io/qt/qtbase.git
            cd qtbase
            ./configure -release -static -sql-sqlite -opensource -confirm-license
            make sub-src
        else
            # Fetch pre-build Qt
            wget https://www.dropbox.com/s/mfw6qwfir62fy9p/macosx-qt5.tar.gz
            tar -xvzf macosx-qt5.tar.gz >& /dev/null
        fi

        cd "$WORKING_DIR/cen64-qt"
    ;;

    'get_quazip')
        wget http://downloads.sourceforge.net/quazip/quazip-0.7.3.tar.gz
        tar -xvzf quazip-0.7.3.tar.gz >& /dev/null
        mv quazip-0.7.3/quazip quazip5
    ;;

    'build')
        ./build-scripts/revision.sh
        $WORKING_DIR/../osx/qtbase/bin/qmake -config release LIBS+="-dead_strip"
        make
    ;;

    'package')
        mkdir -p "build/$TRAVIS_BRANCH"

        hdiutil create -megabytes 20 -fs HFS+ -volname CEN64-Qt "cen64-qt_osx_$VERSION$ARCH"
        hdiutil attach "cen64-qt_osx_$VERSION$ARCH.dmg"

        cp -r CEN64-Qt.app /Volumes/CEN64-Qt/CEN64-Qt.app
        cp resources/README.txt /Volumes/CEN64-Qt/README.txt

        hdiutil detach /Volumes/CEN64-Qt
        hdiutil convert -format UDZO -o "cen64-qt_osx_$VERSION$ARCH.dmg" \
                        -ov "cen64-qt_osx_$VERSION$ARCH.dmg"

        mv "cen64-qt_osx_$VERSION$ARCH.dmg" "build/$TRAVIS_BRANCH/"
    ;;

esac
