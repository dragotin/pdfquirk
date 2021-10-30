cmake_minimum_required(VERSION 3.5)

project(pdfquirk LANGUAGES CXX)

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

find_package(Qt6 COMPONENTS Widgets REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

add_subdirectory(src)

set (XDG_APPS_INSTALL_DIR "share/applications" CACHE STRING "The XDG apps dir")

install(FILES  pdfquirk.appdata.xml DESTINATION share/metainfo RENAME de.volle_kraft_voraus.pdfquirk.appdata.xml)

install(FILES pdfquirk.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} RENAME de.volle_kraft_voraus.pdfquirk.desktop)
ecm_install_icons( ICONS resources/64-apps-pdfquirk.png DESTINATION share/icons)

