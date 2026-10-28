# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
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
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD weact_studio_rp2350b_core CACHE STRING "Board type")

# Personalize
if(PICO_BOARD STREQUAL "weact_studio_rp2350b_core")
    set(PICO_RP2350_A2_SUPPORTED 0)
endif()

# Personalize
if (PICO_COMPILER STREQUAL "pico_arm_clang")
    set(PICO_TOOLCHAIN_PATH "${USERHOME}/Coding/toolchains/LLVM-ET-Arm")
    add_link_options("-fuse-ld=lld")
endif()

set(PICO_CLIB picolibc)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(venti C CXX ASM)

option(VENTI_ENABLE_AUDIO_FOUNDATION "Enable platform-agnostic audio foundation sources" ON)
option(VENTI_BUILD_AUDIO_HOST_SIM "Build host audio simulation executable" OFF)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add FreeRTOS
set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel")
set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/src/include")

if (PICO_RP2040)
    add_subdirectory(
        lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040
        "${CMAKE_CURRENT_BINARY_DIR}/freertos"
    )
elseif (PICO_RP2350 AND NOT PICO_RISCV)
    add_subdirectory(
        "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ"
        "${CMAKE_CURRENT_BINARY_DIR}/freertos"
    )
endif()

set(CMSISCORE "${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS/Core")
set(CMSISDSP_INSTALL OFF)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/CMSIS-DSP/Source bin_dsp)
target_compile_options(CMSISDSP PRIVATE 
    "-O3"
    "-ffast-math"
)

# Add executable. Default name is the project name, version 0.1
add_executable(venti
        lib/SystemView/SYSVIEW/SEGGER_SYSVIEW.c
        lib/SystemView/Sample/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.c
        src/venti.c
        src/pid.c
        src/misc.c
        src/venti_irq.c
        src/tachometer.c
        src/freertos_init.c
        src/gpio_interface.c
        src/usb_interface.c
)

if (VENTI_ENABLE_AUDIO_FOUNDATION)
    target_sources(venti PRIVATE
        src/audio/audio_engine.c
        src/audio/audio_controller.c
        src/audio/audio_decoder_registry.c
    )
endif()

pico_set_program_name(venti "venti")
pico_set_program_version(venti "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(venti 0)
pico_enable_stdio_usb(venti 1)
pico_enable_stdio_rtt(venti 1)

target_compile_definitions(venti PRIVATE RTT_USE_ASM=0)

# Add the standard library to the build
target_link_libraries(venti
        pico_stdlib
        hardware_pwm
        hardware_adc
        FreeRTOS-Kernel-Heap4
        CMSISDSP
        cmsis_core
)

# Add the standard include files to the build
target_include_directories(venti PRIVATE
        src/include
        src/audio/include
        FreeRTOS-Kernel/include
        lib/SystemView/SYSVIEW
        lib/SystemView/Sample/FreeRTOSV11
        lib/SystemView/SEGGER

)

if (PICO_CLIB STREQUAL "picolibc" AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(VENTI_PICOLIBC_SPECS "${USERHOME}/Coding/toolchains/picolibc-1.8.8-13.3.rel1/lib/gcc/arm-none-eabi/13.3.1/picolibc.specs"
        CACHE FILEPATH "Path to the installed GCC picolibc specs file")
    target_compile_options(venti PRIVATE
        "--picolibc-prefix=${USERHOME}/Coding/toolchains/picolibc-1.8.8-13.3.rel1/arm-none-eabi"
        "--specs=${VENTI_PICOLIBC_SPECS}"
    )
    target_compile_options(CMSISDSP PRIVATE
        "--picolibc-prefix=${USERHOME}/Coding/toolchains/picolibc-1.8.8-13.3.rel1/arm-none-eabi"
        "--specs=${VENTI_PICOLIBC_SPECS}"
    )
    target_link_options(venti PRIVATE
        "--picolibc-prefix=${USERHOME}/Coding/toolchains/picolibc-1.8.8-13.3.rel1/arm-none-eabi"
        "--specs=${VENTI_PICOLIBC_SPECS}"
    )
    target_compile_definitions(venti PRIVATE
        PICO_THREAD_LOCAL_PROVIDE_SET_TLS=0
    )
    # Allocate and switch picolibc thread-local storage for each FreeRTOS task.
endif()
# if (CMAKE_C_COMPILER_ID MATCHES "Clang")
target_compile_options(venti PRIVATE
    -fstack-usage
)
# endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_add_extra_outputs(venti)

if (VENTI_BUILD_AUDIO_HOST_SIM)
    add_executable(venti_audio_host_sim
        src/audio/audio_engine.c
        src/audio/audio_controller.c
        src/audio/audio_decoder_registry.c
        src/audio/host_storage_posix.c
        src/audio/host_sim_main.c
    )
    target_include_directories(venti_audio_host_sim PRIVATE
        src/audio/include
    )
    target_compile_definitions(venti_audio_host_sim PRIVATE
        _DEFAULT_SOURCE
    )
endif()
