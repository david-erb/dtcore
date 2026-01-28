cmake_minimum_required(VERSION 3.16)

set(DTCORE_LIBRARY         "${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_library")
set(DTCORE_TESTS           "${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_tests")

set(EXTRA_COMPONENT_DIRS 
    ${DTCORE_LIBRARY}
    ${DTCORE_TESTS}
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(app_esp C)


