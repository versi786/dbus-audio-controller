cmake_minimum_required(VERSION 3.10)

# set project name
project(DbusAudioController VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use package config
find_package(PkgConfig REQUIRED)

PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)
include_directories (${GLIB_INCLUDE_DIRS})
link_directories (${GLIB_LIBRARY_DIRS})

add_executable(dbusAudioController main.cpp)

add_definitions (${GLIB_CFLAGS_OTHER})
target_link_libraries (dbusAudioController ${GLIB_LIBRARIES})
