# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD weact_studio_rp2350b_core CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(venti C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add FreeRTOS
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel")
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(
    "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ"
    "${CMAKE_CURRENT_BINARY_DIR}/freertos"
)

# Add executable. Default name is the project name, version 0.1
add_executable(venti
        venti.c
)

pico_set_program_name(venti "venti")
pico_set_program_version(venti "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(venti 0)
pico_enable_stdio_usb(venti 0)
pico_enable_stdio_rtt(venti 1)
# Pico SDK ships RTT's C implementation, not its optional assembly writer.
target_compile_definitions(venti PRIVATE RTT_USE_ASM=0)

# Add the standard library to the build
target_link_libraries(venti
        pico_stdlib
        FreeRTOS-Kernel-Heap4
)

# Add the standard include files to the build
target_include_directories(venti PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(venti)
