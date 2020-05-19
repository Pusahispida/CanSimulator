#*!
# \file
# \brief CMakeLists.txt foo
#
# Copyright of Link Motion Ltd. All rights reserved.
#
# Contact: info@link-motion.com
#
# \author Matti Lehtimäki <matti.lehtimaki@nomovok.com>
# \author Pauli Oikkonen <pauli.oikkonen@link-motion.com>
#
# any other legal text to be defined later

# Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

find_package(Qt5Widgets CONFIG REQUIRED)

set(EXECUTABLE_NAME ${APPLICATION_NAME}-gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Sources
file(GLOB SOURCES "*.cpp" "*.ui")

add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Use the Widgets module from Qt 5.
target_link_libraries(${EXECUTABLE_NAME} Qt5::Widgets)
target_link_libraries(${EXECUTABLE_NAME} lib${APPLICATION_NAME})

install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION bin)
