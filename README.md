# CEN64-Qt

A customizable cross-platform frontend for [CEN64](https://github.com/n64dev/cen64).

![CEN64-Qt Grid View](https://raw.githubusercontent.com/dh4/cen64-qt/master/resources/demos/main.jpg)


## Getting CEN64-Qt

### Release Builds

Release builds can be found on the [releases](https://github.com/dh4/cen64-qt/releases) page.

Note these are currently only alpha releases versioned with the date. I don't plan to adopt a version scheme until CEN64 is more stable.


### Building (Linux)

First, obtain the source code for CEN64-Qt. You can either clone the repository with git or download an archive of the source code for a specific release from the [releases](https://github.com/dh4/cen64-qt/releases) page.

You'll need to make sure you have cmake, the Qt development libraries and the QuaZip development files installed. On Debian/Ubuntu, this can be accomplished by:

```
# apt install cmake qt6-base-dev libquazip1-qt6-dev
```

Once the needed packages are installed, create the Makefile with cmake and then build with make. Run the following commands from the directory that contains CMakeLists.txt:

```
$ cmake .
$ make
```

If you want to build with Qt5 instead of Qt6, use version 20211016-alpha with the instructions [here](https://github.com/dh4/cen64-qt/blob/20211016-alpha/README.md).

##### Compiling QuaZip statically

You also have the option to compile QuaZip statically. Download the QuaZip sources from GitHub [here](https://github.com/stachenov/quazip/releases). Place the contents of `quazip-<version>/quazip/` in `quazip/` within the project directory. Then run:

```
$ cmake -DLINUX_QUAZIP_STATIC=ON .
$ make
```
