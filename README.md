# CEN64-Qt

A basic cross-platform frontend for CEN64.

## Building

### Linux

You'll need to make sure you have qmake, g++, the Qt development libraries and the Quazip development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt4-qmake g++ libqt4-dev libquazip0-dev
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

```
$ qmake
$ make
```