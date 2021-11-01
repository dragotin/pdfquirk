cmake_minimum_required(VERSION 3.5)

project(pdfquirk LANGUAGES CXX)

option(USE_QT6 "Enable this if you want to use Qt6, instead of Qt5" no)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

include(ECMInstallIcons)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if(USE_QT6)
    find_package(Qt6 COMPONENTS Widgets REQUIRED)
    set(Qt6_Found "Qt6 found flag" CACHE INTERNAL yes)
else(USE_QT6)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif(USE_QT6)

add_subdirectory(src)

set (XDG_APPS_INSTALL_DIR "share/applications" CACHE STRING "The XDG apps dir")

install(FILES  pdfquirk.appdata.xml DESTINATION share/metainfo RENAME de.volle_kraft_voraus.pdfquirk.appdata.xml)

install(FILES pdfquirk.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} RENAME de.volle_kraft_voraus.pdfquirk.desktop)
ecm_install_icons( ICONS resources/64-apps-pdfquirk.png DESTINATION share/icons)

