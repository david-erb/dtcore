cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app_zephyr)

set(DTCORE_DEMOS          ${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_demos)
set(DTCORE_LIBRARY        ${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_library)

# Bring in external object libraries
add_subdirectory(${DTCORE_DEMOS}          ${CMAKE_BINARY_DIR}/dtcore_demos)
add_subdirectory(${DTCORE_LIBRARY}        ${CMAKE_BINARY_DIR}/dtcore_library)

target_include_directories(app PRIVATE
    ${DTCORE_DEMOS}/include
    ${DTCORE_LIBRARY}/include
)

# Use the Zephyr build target (called 'app') – this is created implicitly
target_sources(app PRIVATE
    src/main.c
)
