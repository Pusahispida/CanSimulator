#*!
# \file
# \brief CMakeLists.txt foo
#
# 

# Minimum CMake version
cmake_minimum_required(VERSION 2.8.8)

# Project name language
project(can-simulator-ng CXX)

# global variables
set(APPLICATION_NAME ${PROJECT_NAME})

set(APPLICATION_VERSION_MAJOR "0")
set(APPLICATION_VERSION_MINOR "0")
set(APPLICATION_VERSION_PATCH "1")

set(APPLICATION_VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")

# Compiled flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wextra -Wno-type-limits")

# Add directories
add_subdirectory(lib)
add_subdirectory(cli)

find_package(Qt5 COMPONENTS Core Widgets)
if (Qt5_FOUND)
  add_subdirectory(gui)
else()
  message(WARNING "Could not find Qt, won't build the GUI")
endif()

# Unittesting
find_package(GTest)
if (GTEST_FOUND)
  add_custom_target(tests)
  enable_testing()
  add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()
