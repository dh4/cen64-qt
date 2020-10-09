#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')

[[ -z $ARCH ]] && ARCH=".$(uname -m)"
[[ $ARCH == ".x86_64" ]] && ARCH=""


case "$1" in

    'setup_qt')
        cd "$WORKING_DIR/../"

        if [[ $BUILD_MXE_QT ]]; then
            # Build Qt with mxe
            sudo apt-get update -qq
            sudo apt-get install \
                autoconf automake autopoint bash bison bzip2 cmake flex \
                gettext git g++ gperf intltool libffi-dev libtool \
                libltdl-dev libssl-dev libxml-parser-perl make openssl \
                p7zip-full patch perl pkg-config python ruby scons sed \
                unzip wget xz-utils libtool-bin lzip libgdk-pixbuf2.0-dev
            git clone https://github.com/mxe/mxe.git
            cd mxe
            make qtbase
        else
            # Fetch pre-built mxe Qt
            wget https://www.dropbox.com/s/jr6l4lnixizqtln/travis-mxe-qt5.tar.gz
            tar -xvzf travis-mxe-qt5.tar.gz > /dev/null
        fi

        cd "$WORKING_DIR/cen64-qt"
    ;;

    'get_quazip')
        wget http://downloads.sourceforge.net/quazip/quazip-0.7.3.tar.gz
        tar -xvzf quazip-0.7.3.tar.gz > /dev/null
        mv quazip-0.7.3/quazip quazip5
    ;;

    'build')
        export PATH="$PATH:$WORKING_DIR/../mxe/usr/bin"

        ./build-scripts/revision.sh
        i686-w64-mingw32.static-qmake-qt5
        make
    ;;

    'package')
        mkdir -p "build/$TRAVIS_BRANCH"

        mv release/cen64-qt.exe resources/README.txt .
        zip "build/$TRAVIS_BRANCH/cen64-qt_win_$VERSION$ARCH.zip" cen64-qt.exe README.txt
    ;;

esac
