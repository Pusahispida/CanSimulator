#*!
# \file
# \brief CMakeLists.txt foo
# Include modules
include(FindPkgConfig)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/can-dbcparser/header)

# Sources
file(GLOB SOURCES "*.cpp")
file(GLOB CAN_DBCPARSER_SOURCES "can-dbcparser/*.cpp")

pkg_check_modules (JSON jansson REQUIRED)
pkg_check_modules (SOCKETCAN libsocketcan REQUIRED)
include_directories(${JSON_INCLUDE_DIRS} ${SOCKETCAN_INCLUDE_DIRS})
link_libraries(${JSON_LIBRARY_DIRS} ${SOCKETCAN_LIBRARY_DIRS} -lcap)

add_library(lib${APPLICATION_NAME} ${SOURCES} ${CAN_DBCPARSER_SOURCES})

set_target_properties(lib${APPLICATION_NAME} PROPERTIES OUTPUT_NAME ${APPLICATION_NAME})

target_link_libraries(lib${APPLICATION_NAME} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
