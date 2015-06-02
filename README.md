# CEN64-Qt

A basic cross-platform frontend for CEN64.

## Building

### Linux

You'll need to make sure you have qmake, g++, the Qt development libraries and the Quazip development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt4-qmake g++ libqt4-dev libquazip-dev libqt4-sql-sqlite
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

```
$ qmake-qt4
$ make
```

#### Building with Qt5

Install the Qt5 dependencies instead:

```
# apt-get install qt5-qmake g++ qtbase5-dev libquazip-qt5-dev libqt5sql5-sqlite
```

On a Debian-based distribution with libquazip-qt5-dev installed, you'll need to change -lquazip to -lquazip-qt5 near the end of cen64-qt.pro.

```
$ qmake-qt5
$ make
```