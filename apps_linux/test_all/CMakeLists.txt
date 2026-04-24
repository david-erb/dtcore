cmake_minimum_required(VERSION 3.20)
project(app C)

include(${CMAKE_CURRENT_LIST_DIR}/../../dthelpers.cmake)
dthelpers_resolve(DTCORE_DIR "dtcore repo directory")

add_subdirectory(${DTCORE_DIR}/dtcore_tests           ${CMAKE_BINARY_DIR}/dtcore_tests)
add_subdirectory(${DTCORE_DIR}/dtcore_library         ${CMAKE_BINARY_DIR}/dtcore_library)

add_executable(app src/main.c)

target_link_libraries(app
    PRIVATE
        dtcore_tests
        dtcore_library
)
