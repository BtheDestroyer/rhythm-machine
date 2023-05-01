cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rhythm_machine C CXX ASM)

add_subdirectory(libs/FatFS_SD/FatFs_SPI)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rhythm_machine
        "src/audio.cpp"
        "src/lcd.cpp"
        "src/machine.cpp"
        "src/main.cpp"
        "src/sd.cpp"
        )

target_include_directories(rhythm_machine
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/inc
        )


pico_set_program_name(rhythm_machine "rhythm_machine")
pico_set_program_version(rhythm_machine "0.1")

pico_generate_pio_header(rhythm_machine ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

pico_enable_stdio_uart(rhythm_machine 1)
pico_enable_stdio_usb(rhythm_machine 0)

# Add the standard library to the build
target_link_libraries(rhythm_machine pico_stdlib)

# Add any user requested libraries
target_link_libraries(rhythm_machine
        FatFs_SPI
        hardware_clocks
        hardware_dma
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_timer
        )

pico_add_extra_outputs(rhythm_machine)
