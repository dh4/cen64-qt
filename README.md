# CEN64-Qt

A basic cross-platform frontend for CEN64.

## Building

### Linux

You'll need to make sure you have qmake, g++, the Qt development libraries and the QuaZIP development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt5-qmake g++ qtbase5-dev libquazip-qt5-dev libqt5sql5-sqlite
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

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

Then create the Makefile with qmake and build with make:

```
$ qmake-qt4
$ make
```