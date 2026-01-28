cmake_minimum_required(VERSION 3.20)

# --- Tell CMake we're cross-compiling (prevents try-run) ---
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# --- Select the Pico board BEFORE importing the SDK ---
# Uses Docker ENV 
set(PICO_BOARD $ENV{PICO_BOARD})

# --- Import the pico-sdk (uses ENV PICO_SDK_PATH from the container) ---
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(app C CXX ASM)
pico_sdk_init()


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_demos      ${CMAKE_BINARY_DIR}/dtcore_demos)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../dtcore_library    ${CMAKE_BINARY_DIR}/dtcore_library)

# main() lives in the app
add_executable(app
    src/main.c
)

pico_enable_stdio_usb(app 1)
pico_enable_stdio_uart(app 0)

target_link_libraries(app
    PRIVATE
        dtcore_demos
        dtcore_library
)

# Make .uf2 / .bin / .elf
pico_add_extra_outputs(app)