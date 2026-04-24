cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/../../dthelpers.cmake)
dthelpers_resolve(DTCORE_DIR "dtcore repo directory")

set(EXTRA_COMPONENT_DIRS 
    ${DTCORE_DIR}/dtcore_demos
    ${DTCORE_DIR}/dtcore_library
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(app_esp C)


