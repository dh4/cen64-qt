# CEN64-Qt [![Build Status](https://travis-ci.org/dh4/cen64-qt.svg?branch=master)](https://travis-ci.org/dh4/cen64-qt)

A basic cross-platform frontend for [CEN64](http://cen64.com/).

![CEN64-Qt Grid View](https://dl.dropboxusercontent.com/u/232085155/cen64-qt/github.jpg)


## Getting CEN64-Qt

### Release Builds

Release builds can be found on the [releases](https://github.com/dh4/cen64-qt/releases) page.

Note these are currently only alpha releases versioned with the date. I don't plan to adopt a version scheme until CEN64 is more stable.

### Development Builds

Automatic builds of the latest git commit can be downloaded here:  
Linux: [cen64-qt_linux_git-latest.tar.gz](https://s3.amazonaws.com/dh4/cen64-qt/master/cen64-qt_linux_git-latest.tar.gz)  
Windows: [cen64-qt_win_git-latest.zip](https://s3.amazonaws.com/dh4/cen64-qt/master/cen64-qt_win_git-latest.zip)  
OSX: [cen64-qt_osx_git-latest.dmg](https://s3.amazonaws.com/dh4/cen64-qt/master/cen64-qt_osx_git-latest.dmg)

Be aware that these may contain bugs not present in the release builds.

### Building (Linux)

First, obtain the source code for CEN64-Qt. You can either clone the repository with git or download an archive of the source code for a specific release from the [releases](https://github.com/dh4/cen64-qt/releases) page.

You'll need to make sure you have qmake, g++, the Qt development libraries and the QuaZIP development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt5-qmake g++ qtbase5-dev libquazip-qt5-dev libqt5sql5-sqlite
```

Once the needed packages are installed, create the Makefile with qmake and then build with make. Run the following commands from the directory that contains cen64-qt.pro:

```
$ qmake
$ make
```

If qmake returns "qmake: could not find a Qt installation of ''", you can try running `QT_SELECT=qt5 qmake` or `/usr/lib/x86_64-linux-gnu/qt5/bin/qmake`. Some distributions also contain a `qmake-qt5` symlink.

#### Building with Qt4

Install the Qt4 dependencies instead. On Debian/Ubuntu:

```
# apt-get install qt4-qmake g++ libqt4-dev libquazip-dev libqt4-sql-sqlite
```

Then create the Makefile with qmake and build with make. Run the following commands from the directory that contains cen64-qt.pro:

```
$ qmake-qt4
$ make
```

#### Compiling QuaZIP statically

You also have the option to compile QuaZIP statically. Download the QuaZIP sources from Sourceforge. Place the contents of `quazip-<version>/quazip/` in `quazip/` within the project directory. Then run:

```
$ qmake CONFIG+=linux_quazip_static
$ make
```

You will see warnings after the qmake step if the QuaZIP sources are in the wrong place.
