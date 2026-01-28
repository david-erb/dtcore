cmake_minimum_required(VERSION 3.20)
project(app C)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_library    ${CMAKE_BINARY_DIR}/dtcore_library)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_tests      ${CMAKE_BINARY_DIR}/dtcore_tests)

# main() lives in the app
add_executable(app
    src/main.c
)

target_link_libraries(app
    PRIVATE
        dtcore_library
        dtcore_tests
)
