# source discovery (safe across normal configure + script/find-package modes)
set(DTCORE_GLOB_PATTERNS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dt*.c
)

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE AND NOT DEFINED CMAKE_FIND_PACKAGE_NAME)
    file(GLOB DTCORE_LIBRARY_SOURCES
        CONFIGURE_DEPENDS
        ${DTCORE_GLOB_PATTERNS}
    )
else()
    # Script mode / find-package mode: CONFIGURE_DEPENDS is not allowed.
    file(GLOB DTCORE_LIBRARY_SOURCES
        ${DTCORE_GLOB_PATTERNS}
    )
endif()

# Exclude files added explicitly in platform blocks
list(REMOVE_ITEM DTCORE_LIBRARY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dtpicosdk_helper.c
)


set(DTCORE_LIBRARY_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- Zephyr -------------------------------------------------
if (ZEPHYR_BASE OR DEFINED Zephyr_FOUND)
    target_sources(app PRIVATE ${DTCORE_LIBRARY_SOURCES})
    target_include_directories(app PRIVATE ${DTCORE_LIBRARY_INCLUDE})
    
# ---- ESP-IDF -------------------------------------------------
elseif(DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS ${DTCORE_LIBRARY_SOURCES}
        INCLUDE_DIRS include
    )

# ---- Pico SDK -------------------------------------------------
elseif(DEFINED PICO_BOARD)
    add_library(dtcore_library STATIC
        ${DTCORE_LIBRARY_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dtpicosdk_helper.c
    )

    target_link_libraries(dtcore_library
        PUBLIC
            pico_stdlib
    )

    target_include_directories(dtcore_library
        PUBLIC
            ${DTCORE_LIBRARY_INCLUDE}
    )

# ---- Linux -------------------------------------------------
else()
    add_library(dtcore_library STATIC
        ${DTCORE_LIBRARY_SOURCES}
    )

    target_include_directories(dtcore_library
        PUBLIC
            ${DTCORE_LIBRARY_INCLUDE}
    )
endif()
