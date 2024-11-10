cmake_minimum_required(VERSION 3.16)
project(cen64-qt VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

option(LINUX_QUAZIP_STATIC "Link QuaZip statically")

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui Network Sql Widgets Xml)

qt_standard_project_setup()

if(MACOS OR WIN32 OR LINUX_QUAZIP_STATIC)
    find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
    file(GLOB_RECURSE QUAZIP_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "quazip/*.c" "quazip/*.cpp")
endif()

if(UNIX AND NOT MACOS AND NOT LINUX_QUAZIP_STATIC)
    find_package(QuaZip-Qt6 REQUIRED)
endif()

qt_add_executable(cen64-qt WIN32 MACOSX_BUNDLE
    src/common.cpp src/common.h
    src/dialogs/aboutdialog.cpp src/dialogs/aboutdialog.h
    src/dialogs/downloaddialog.cpp src/dialogs/downloaddialog.h
    src/dialogs/logdialog.cpp src/dialogs/logdialog.h
    src/dialogs/settingsdialog.cpp src/dialogs/settingsdialog.h src/dialogs/settingsdialog.ui
    src/dialogs/v64converter.cpp src/dialogs/v64converter.h
    src/emulation/emulatorhandler.cpp src/emulation/emulatorhandler.h
    src/global.h
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/roms/romcollection.cpp src/roms/romcollection.h
    src/roms/thegamesdbscraper.cpp src/roms/thegamesdbscraper.h
    src/views/ddview.cpp src/views/ddview.h
    src/views/gridview.cpp src/views/gridview.h
    src/views/listview.cpp src/views/listview.h
    src/views/tableview.cpp src/views/tableview.h
    src/views/widgets/clickablewidget.cpp src/views/widgets/clickablewidget.h
    src/views/widgets/treewidgetitem.cpp src/views/widgets/treewidgetitem.h
    ${QUAZIP_SOURCES}
)
target_link_libraries(cen64-qt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Sql
    Qt::Widgets
    Qt::Xml
)


# Resources:
set(cen64qt_resource_files
    "resources/images/cen64.png"
    "resources/images/no-cart.png"
    "resources/images/not-found.png"
    "resources/locale/cen64-qt_fr.qm"
    "resources/locale/cen64-qt_ru.qm"
    "resources/other/LICENSE"
    "resources/other/VERSION"
)

qt_add_resources(cen64-qt "cen64qt"
    PREFIX
        "/"
    BASE
        "resources"
    FILES
        ${cen64qt_resource_files}
)

if(MACOS OR WIN32 OR LINUX_QUAZIP_STATIC)
    target_link_libraries(cen64-qt PRIVATE
        z
        Qt6::Core5Compat
    )
endif()

if(UNIX AND NOT MACOS AND NOT LINUX_QUAZIP_STATIC)
    target_link_libraries(cen64-qt PRIVATE
        QuaZip::QuaZip
    )
endif()

install(TARGETS cen64-qt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET cen64-qt
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
