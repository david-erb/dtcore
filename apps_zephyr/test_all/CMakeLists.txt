cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app_zephyr)

include(${CMAKE_CURRENT_LIST_DIR}/../../dthelpers.cmake)
dthelpers_resolve(DTCORE_DIR "dtcore repo directory")

add_subdirectory(${DTCORE_DIR}/dtcore_tests          ${CMAKE_BINARY_DIR}/dtcore_tests)
add_subdirectory(${DTCORE_DIR}/dtcore_library        ${CMAKE_BINARY_DIR}/dtcore_library)

target_link_libraries(app PRIVATE 
    dtcore_tests
    dtcore_library)

target_sources(app PRIVATE
    src/main.c
)