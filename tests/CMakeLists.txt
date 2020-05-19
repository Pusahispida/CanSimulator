#*!
# \file
# \brief CMakeLists.txt foo

cmake_minimum_required(VERSION 2.8)

find_package(GTest REQUIRED)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../cli
    ${GTEST_INCLUDE_DIRS}
    )

include(FindPkgConfig)
pkg_check_modules (JSON jansson REQUIRED)
pkg_check_modules (SOCKETCAN libsocketcan REQUIRED)
include_directories(${JSON_INCLUDE_DIRS} ${SOCKETCAN_INCLUDE_DIRS})
link_libraries(${JSON_LIBRARY_DIRS} ${SOCKETCAN_LIBRARY_DIRS} -lcap)

configure_file(tests.cfg tests.cfg COPYONLY)
configure_file(tests.dbc tests.dbc COPYONLY)
configure_file(tests.asc tests.asc COPYONLY)
configure_file(tests_missing.asc tests_missing.asc COPYONLY)
configure_file(tests_relative.asc tests_relative.asc COPYONLY)
configure_file(tests_relative_second.asc tests_relative_second.asc COPYONLY)
configure_file(tests_second.asc tests_second.asc COPYONLY)

add_executable(test_cli main.cpp
    ${TESTS}
    )
target_link_libraries(test_cli ${GTEST_LIBRARIES} pthread)
add_test("CommandLineIinterface" test_cli)

FILE(GLOB CANFRAME_TESTS "test_LIB_canframe.cpp")

add_executable(test_canframe main.cpp
    ${CANFRAME_TESTS}
    )
target_link_libraries(test_canframe ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("CANFrame" test_canframe)

FILE(GLOB CONFIG_TESTS "test_LIB_configuration.cpp")

add_executable(test_configuration main.cpp
    ${CONFIG_TESTS}
    )
target_link_libraries(test_configuration ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("Configuration" test_configuration)

FILE(GLOB ERRORFRAME_TESTS "test_LIB_errorframe.cpp")

add_executable(test_errorframe main.cpp
    ${ERRORFRAME_TESTS}
    )
target_link_libraries(test_errorframe ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("ErrorFrame" test_errorframe)

FILE(GLOB QUEUE_TESTS "test_LIB_queue.cpp")

add_executable(test_queue main.cpp
    ${QUEUE_TESTS}
    )
target_link_libraries(test_queue ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("Queue" test_queue)

FILE(GLOB FLOOD_TESTS "test_CLI_floodmode.cpp")

add_executable(test_flood main.cpp
    ${FLOOD_TESTS}
    )
target_link_libraries(test_flood ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("Flood" test_flood)

FILE(GLOB ASCPARSER_TESTS "test_LIB_ascreader.cpp")

add_executable(test_ascreader main.cpp
    ${ASCPARSER_TESTS}
    )
target_link_libraries(test_ascreader ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES})
add_test("ASCReader" test_ascreader)

FILE(GLOB METRICS_TESTS "test_LIB_metrics.cpp")

add_executable(test_metrics main.cpp
    ${METRICS_TESTS}
    )
target_link_libraries(test_metrics ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("Metrics" test_metrics)

FILE(GLOB FILTER_TESTS "test_LIB_filters.cpp")

add_executable(test_filters main.cpp
    ${FILTER_TESTS}
    )
target_link_libraries(test_filters ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("Filters" test_filters)

FILE(GLOB CORE_TESTS "test_LIB_cansimulatorcore.cpp")

add_executable(test_cansimulatorcore main.cpp
    ${CORE_TESTS}
    )
target_link_libraries(test_cansimulatorcore ${GTEST_LIBRARIES} pthread ${JSON_LIBRARIES} ${SOCKETCAN_LIBRARIES})
add_test("CANSimulatorCore" test_cansimulatorcore)

add_dependencies(tests test_cli test_configuration test_canframe test_errorframe test_queue test_flood test_ascreader test_metrics test_filters test_cansimulatorcore)
