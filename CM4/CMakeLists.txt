cmake_minimum_required(VERSION 3.16)

# Load toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake)

project(pocat_moon_sw)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MAIN_SOURCE_FILE main.c)

set(MCU_FAMILY STM32MP1xx)   # MCU Family
set(MCU_MODEL STM32MP157Fxx)  # MCU Model

set(CPU_PARAMETERS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32mp157facx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/STM32MP157FACX_RAM.ld)

set(EXECUTABLE ${CMAKE_PROJECT_NAME}.elf)
enable_language(C ASM CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20) # changed from 17 to 20 (radiolib is in c++20)

# Include folders

set(CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32MP1xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32MP1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
)

set(PROJECT_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Common
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Hal
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Interrupts
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Subsystem
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Tasks
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Utils/Radiolib
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Subsystems
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Semtech/Inc
)

set(FREERTOS_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/CMSIS_RTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM4F
)

set(RADIOLIB_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/RadioLib/src
)
	
# Gather sources #MIRAR
#file(GLOB_RECURSE STM32CUBEMX_SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c
#)

file(GLOB STM32_HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32MP1xx_HAL_Driver/Src/*.c
)

file(GLOB STM32_CMSIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32MP1xx/Source/*.c
)

set(STM32CUBEMX_SOURCES
    ${STM32_HAL_SOURCES}
    ${STM32_CMSIS_SOURCES}
)

file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Semtech/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Utils/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Utils/Radiolib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/Subsystems/*.c
)

#file(GLOB_RECURSE FREERTOS_SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/*.c
#)

file(GLOB FREERTOS_KERNEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/*.c
)

file(GLOB FREERTOS_PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM4F/*.c
)

file(GLOB FREERTOS_CMSIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/CMSIS_RTOS/*.c
)

set(FREERTOS_SOURCES
    ${FREERTOS_KERNEL_SOURCES}
    ${FREERTOS_PORT_SOURCES}
    ${FREERTOS_CMSIS_SOURCES}
)

file(GLOB_RECURSE RADIOLIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/RadioLib/src/modules/*.cpp
    ${CMAKE_SOURCE_DIR}/Core/Inc/RadioLib/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/RadioLib/src/protocols/*.cpp
    # Add other needed source directories
)

# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES}
    ${FREERTOS_SOURCES}
    ${PROJECT_SOURCES}
    ${RADIOLIB_SOURCES}
    ${STARTUP_SCRIPT}
)


# Embedded macros(defines)
target_compile_definitions(${EXECUTABLE} PRIVATE
    STM32MP157Fxx   
    USE_HAL_DRIVER
    CORE_CM4
)

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${FREERTOS_INCLUDE_DIRECTORIES}
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${RADIOLIB_INCLUDE_DIRECTORIES}
)

# Ensure Debug build type is set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Compiler options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    $<$<CONFIG:Debug>:-Og -g3 -ggdb>	# Ensure proper debugger flags
    $<$<CONFIG:Release>:-O2 -g0>	# Optimize in release mode
)


# Ensure debugging symbols in the final executable
target_link_options(${EXECUTABLE} PRIVATE
    -T${MCU_LINKER_SCRIPT}
    ${CPU_PARAMETERS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -Wl,--print-memory-usage
    -g3 -ggdb  # Ensures GDB debugging info
)

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>
)

