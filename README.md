# CEN64-Qt

A basic cross-platform frontend for CEN64.

## Building

### Linux

You'll need to make sure you have qmake, g++ and the Qt development libraries installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt-get install qt4-qmake g++ libqt4-dev
```

Once the needed packages are installed, create the Makefile with qmake and then build with make:

```
$ qmake
$ make
```