# dtcore_library/CMakeLists.txt

include(${CMAKE_CURRENT_LIST_DIR}/../dthelpers.cmake)
dthelpers_glob(MY_SOURCES dt*.c)

# Exclude files added explicitly in platform blocks
list(REMOVE_ITEM MY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dtpicosdk_helper.c
)
 
# ---- Zephyr -------------------------------------------------
if (ZEPHYR_BASE OR DEFINED Zephyr_FOUND)

    zephyr_library_named(dtcore_library)

    zephyr_library_sources(${MY_SOURCES})
    
    zephyr_library_include_directories(
        ${DTCORE_DIR}/dtcore_library/include
    )    

# ---- ESP-IDF -------------------------------------------------
elseif(DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS ${MY_SOURCES}
        INCLUDE_DIRS include
    )

# ---- Pico SDK -------------------------------------------------
elseif(DEFINED PICO_BOARD)
    add_library(dtcore_library STATIC
        ${MY_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dtpicosdk_helper.c
    )

    target_link_libraries(dtcore_library
        PUBLIC
            pico_stdlib
    )

    target_include_directories(dtcore_library
        PUBLIC
            ${DTCORE_DIR}/dtcore_library/include
    )

# ---- Linux -------------------------------------------------
else()
    add_library(dtcore_library STATIC ${MY_SOURCES})

    # Publish this lib's public headers
    target_include_directories(dtcore_library PUBLIC ${DTCORE_DIR}/dtcore_library/include)
    
    # Depend on other libraries so their headers & link deps propagate.
    target_link_libraries(dtcore_library PUBLIC m)
endif()
