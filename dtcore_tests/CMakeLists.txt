if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE AND NOT DEFINED CMAKE_FIND_PACKAGE_NAME)
    file(GLOB MY_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/test_*.c)
else()
    file(GLOB MY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/test_*.c)
endif()

# ---- Zephyr -------------------------------------------------
if (ZEPHYR_BASE OR DEFINED Zephyr_FOUND)
    zephyr_library_named(dtcore_tests)
    zephyr_library_sources(${MY_SOURCES})
    target_include_directories(dtcore_tests PUBLIC ${DTCORE_DIR}/dtcore_tests/include ${DTCORE_DIR}/dtcore_library/include)
    target_link_libraries(dtcore_tests PUBLIC dtcore_library)
    
# ---- ESP-IDF -------------------------------------------------
elseif(DEFINED ENV{IDF_PATH})
    idf_component_register(
        SRCS ${MY_SOURCES}
        INCLUDE_DIRS include
        REQUIRES dtcore_library
    )

# ---- Pico SDK -------------------------------------------------
elseif(DEFINED PICO_BOARD)
    add_library(dtcore_tests STATIC
        ${MY_SOURCES}
    )

    target_include_directories(dtcore_tests
        PUBLIC
            ${DTCORE_DIR}/dtcore_tests/include
            ${DTCORE_DIR}/dtcore_library/include
    )

    target_link_libraries(dtcore_tests
        PUBLIC
            dtcore_library
    )

# ---- Linux -------------------------------------------------
else()
    add_library(dtcore_tests STATIC ${MY_SOURCES})
    target_include_directories(dtcore_tests PUBLIC ${DTCORE_DIR}/dtcore_tests/include)
    target_link_libraries(dtcore_tests PUBLIC dtcore_library)

endif()
