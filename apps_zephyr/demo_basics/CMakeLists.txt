# demo_basics/CMakeLists.txt

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app_zephyr)

include(${CMAKE_CURRENT_LIST_DIR}/../../dthelpers.cmake)
dthelpers_resolve(DTCORE_DIR "dtcore repo directory")

add_subdirectory(${DTCORE_DIR}/dtcore_demos          ${CMAKE_BINARY_DIR}/dtcore_demos)
add_subdirectory(${DTCORE_DIR}/dtcore_library        ${CMAKE_BINARY_DIR}/dtcore_library)

target_include_directories(app PRIVATE
    ${DTCORE_DIR}/dtcore_demos/include
    ${DTCORE_DIR}/dtcore_library/include
)   

target_link_libraries(app PRIVATE 
    dtcore_demos
    dtcore_library)

target_sources(app PRIVATE
    src/main.c
)