cmake_minimum_required(VERSION 3.10)

# set project name
project(DbusAudioController VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Use package config
find_package(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)
include_directories (${GLIB_INCLUDE_DIRS})
link_directories (${GLIB_LIBRARY_DIRS})


PKG_CHECK_MODULES(DBUS REQUIRED dbus-1)
include_directories (${DBUS_INCLUDE_DIRS})
link_directories (${DBUS_LIBRARY_DIRS})

PKG_CHECK_MODULES(DBUS-GLIB REQUIRED dbus-glib-1)
include_directories (${DBUS-GLIB_INCLUDE_DIRS})
link_directories (${DBUS-GLIB_LIBRARY_DIRS})

PKG_CHECK_MODULES(GIO REQUIRED gio-2.0)
include_directories (${GIO_INCLUDE_DIRS})
link_directories (${GIO_LIBRARY_DIRS})

PKG_CHECK_MODULES(GIO-unix REQUIRED gio-unix-2.0)
include_directories (${GIO-unix_INCLUDE_DIRS})
link_directories (${GIO-unix_LIBRARY_DIRS})

SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.65 REQUIRED COMPONENTS program_options log)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )



add_executable(dbusAudioController main.cpp generated_mediaplayer2.c)
target_compile_options(dbusAudioController PRIVATE -Werror -Wall -Wextra -Wno-error=unused-parameter)

add_definitions (${GLIB_CFLAGS_OTHER})
target_link_libraries (dbusAudioController ${GLIB_LIBRARIES})

add_definitions (${DBUDBUS_CFLAGS_OTHER})
target_link_libraries (dbusAudioController ${DBUS_LIBRARIES})

add_definitions (${GIO_CFLAGS_OTHER})
target_link_libraries (dbusAudioController ${GIO_LIBRARIES})

add_definitions (${GIO-unix_CFLAGS_OTHER})
target_link_libraries (dbusAudioController ${GIO-unix_LIBRARIES})

target_link_libraries (dbusAudioController  ${Boost_LIBRARIES} )
