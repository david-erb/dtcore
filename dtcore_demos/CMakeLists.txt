# source discovery (safe across normal configure + script/find-package modes)
set(DTCORE_DEMO_GLOB_PATTERNS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/demo_*.c
)

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE AND NOT DEFINED CMAKE_FIND_PACKAGE_NAME)
    file(GLOB DTCORE_DEMO_SOURCES
        CONFIGURE_DEPENDS
        ${DTCORE_DEMO_GLOB_PATTERNS}
    )
else()
    # Script mode / find-package mode: CONFIGURE_DEPENDS is not allowed.
    file(GLOB DTCORE_DEMO_SOURCES
        ${DTCORE_DEMO_GLOB_PATTERNS}
    )
endif()

set(DTCORE_DEMO_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DTCORE_LIBRARY_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../dtcore_library/include
)

# ---- Zephyr -------------------------------------------------
if (ZEPHYR_BASE OR DEFINED Zephyr_FOUND)
    target_sources(app PRIVATE ${DTCORE_DEMO_SOURCES})
    target_include_directories(app PRIVATE
        ${DTCORE_DEMO_INCLUDE}
        ${DTCORE_LIBRARY_INCLUDE}
    )
    
# ---- ESP-IDF -------------------------------------------------
elseif(DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS ${DTCORE_DEMO_SOURCES}
        INCLUDE_DIRS include
        REQUIRES dtcore_library
    )

# ---- Pico SDK -------------------------------------------------
elseif(DEFINED PICO_BOARD)
    add_library(dtcore_demos STATIC
        ${DTCORE_DEMO_SOURCES}
    )

    target_include_directories(dtcore_demos
        PUBLIC
            ${DTCORE_DEMO_INCLUDE}
            ${DTCORE_LIBRARY_INCLUDE}
    )

    target_link_libraries(dtcore_demos
        PUBLIC
            dtcore_library
    )

# ---- Linux -------------------------------------------------
else()
    add_library(dtcore_demos STATIC
        ${DTCORE_DEMO_SOURCES}
    )

    target_include_directories(dtcore_demos
        PUBLIC
            ${DTCORE_DEMO_INCLUDE}
            ${DTCORE_LIBRARY_INCLUDE}
    )

    target_link_libraries(dtcore_demos
        PUBLIC
            dtcore_library
    )

endif()
