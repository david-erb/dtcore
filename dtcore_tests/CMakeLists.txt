# source discovery (safe across normal configure + script/find-package modes)
set(DTCORE_TEST_GLOB_PATTERNS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_*.c
)

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE AND NOT DEFINED CMAKE_FIND_PACKAGE_NAME)
    file(GLOB DTCORE_TEST_SOURCES
        CONFIGURE_DEPENDS
        ${DTCORE_TEST_GLOB_PATTERNS}
    )
else()
    # Script mode / find-package mode: CONFIGURE_DEPENDS is not allowed.
    file(GLOB DTCORE_TEST_SOURCES
        ${DTCORE_TEST_GLOB_PATTERNS}
    )
endif()

set(DTCORE_TEST_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(DTCORE_LIBRARY_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../dtcore_library/include
)

# ---- Zephyr -------------------------------------------------
if (ZEPHYR_BASE OR DEFINED Zephyr_FOUND)
    target_sources(app PRIVATE ${DTCORE_TEST_SOURCES})
    target_include_directories(app PRIVATE
        ${DTCORE_TEST_INCLUDE}
        ${DTCORE_LIBRARY_INCLUDE}
    )
    
# ---- ESP-IDF -------------------------------------------------
elseif(DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS ${DTCORE_TEST_SOURCES}
        INCLUDE_DIRS include
        REQUIRES dtcore_library
    )

# ---- Pico SDK -------------------------------------------------
elseif(DEFINED PICO_BOARD)
    add_library(dtcore_tests STATIC
        ${DTCORE_TEST_SOURCES}
    )

    target_include_directories(dtcore_tests
        PUBLIC
            ${DTCORE_TEST_INCLUDE}
            ${DTCORE_LIBRARY_INCLUDE}
    )

    target_link_libraries(dtcore_tests
        PUBLIC
            dtcore_library
    )

# ---- Linux -------------------------------------------------
else()
    add_library(dtcore_tests STATIC
        ${DTCORE_TEST_SOURCES}
    )

    target_include_directories(dtcore_tests
        PUBLIC
            ${DTCORE_TEST_INCLUDE}
            ${DTCORE_LIBRARY_INCLUDE}
    )

    target_link_libraries(dtcore_tests
        PUBLIC
            dtcore_library
    )

endif()
