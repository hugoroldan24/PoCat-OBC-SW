cmake_minimum_required(VERSION 3.16)

# Load toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake)

# Set project root
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

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

set(STARTUP_SCRIPT ${PROJECT_ROOT}/Core/Startup/startup_stm32mp157facx.s)
set(MCU_LINKER_SCRIPT ${PROJECT_ROOT}/STM32MP157FACX_RAM.ld)

set(EXECUTABLE ${CMAKE_PROJECT_NAME}.elf)
enable_language(C ASM CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20) # changed from 17 to 20 (radiolib is in c++20)

# Include folders

set(CUBEMX_INCLUDE_DIRECTORIES
    ${PROJECT_ROOT}/Drivers/STM32MP1xx_HAL_Driver/Inc
    ${PROJECT_ROOT}/Drivers/STM32MP1xx_HAL_Driver/Inc/Legacy
    ${PROJECT_ROOT}/Drivers/CMSIS/Device/ST/STM32MP1xx/Include
    ${PROJECT_ROOT}/Drivers/CMSIS/Include
)

set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_ROOT}/Core/Inc/
    ${PROJECT_ROOT}/Application/
    ${PROJECT_ROOT}/Application/Services/Events
    ${PROJECT_ROOT}/Application/Services/Events/Handlers
    ${PROJECT_ROOT}/Application/Services/Manager
    ${PROJECT_ROOT}/Application/Subsystems
    ${PROJECT_ROOT}/Application/Utils
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Common
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Hal
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Interrupts
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Subsystem
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc/Tasks
)

set(FREERTOS_INCLUDE_DIRECTORIES
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/include
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/CMSIS_RTOS
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM4F
)

set(RADIOLIB_INCLUDE_DIRECTORIES
    ${PROJECT_ROOT}/Core/Inc/RadioLib/src
)
	
# Gather sources #MIRAR
#file(GLOB_RECURSE STM32CUBEMX_SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c
#)

file(GLOB STM32_HAL_SOURCES
    ${PROJECT_ROOT}/Drivers/STM32MP1xx_HAL_Driver/Src/*.c
)

file(GLOB STM32_CMSIS_SOURCES
    ${PROJECT_ROOT}/Drivers/CMSIS/Device/ST/STM32MP1xx/Source/*.c
)

set(STM32CUBEMX_SOURCES
    ${STM32_HAL_SOURCES}
    ${STM32_CMSIS_SOURCES}
)

file(GLOB_RECURSE PROJECT_SOURCES
    ${PROJECT_ROOT}/Core/Src/*.c
    ${PROJECT_ROOT}/Application/Src/*.c
    ${PROJECT_ROOT}/Application/Src/Services/Events/*.c
    ${PROJECT_ROOT}/Application/Src/Services/Events/Handlers/*.c
    ${PROJECT_ROOT}/Application/Src/Services/Manager/*.c
    ${PROJECT_ROOT}/Application/Src/Subsystems/*.c
    ${PROJECT_ROOT}/Application/Src/Utils/*.c
)

#file(GLOB_RECURSE FREERTOS_SOURCES
#    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/*.c
#)

file(GLOB FREERTOS_KERNEL_SOURCES
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/*.c
)

file(GLOB FREERTOS_PORT_SOURCES
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/portable/GCC/ARM_CM4F/*.c
)

file(GLOB FREERTOS_CMSIS_SOURCES
    ${PROJECT_ROOT}/Middlewares/Third_Party/FreeRTOS/CMSIS_RTOS/*.c
)

set(FREERTOS_SOURCES
    ${FREERTOS_KERNEL_SOURCES}
    ${FREERTOS_PORT_SOURCES}
    ${FREERTOS_CMSIS_SOURCES}
)

file(GLOB_RECURSE RADIOLIB_SOURCES
    ${PROJECT_ROOT}/Core/Inc/RadioLib/src/modules/*.cpp
    ${PROJECT_ROOT}/Core/Inc/RadioLib/src/*.cpp
    ${PROJECT_ROOT}/Core/Inc/RadioLib/src/protocols/*.cpp
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

