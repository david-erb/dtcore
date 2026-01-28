cmake_minimum_required(VERSION 3.16)

set(DTCORE_LIBRARY         "${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_library")
set(DTCORE_DEMOS           "${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_demos")

set(EXTRA_COMPONENT_DIRS 
    ${DTCORE_LIBRARY}
    ${DTCORE_DEMOS}
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(app_esp C)


