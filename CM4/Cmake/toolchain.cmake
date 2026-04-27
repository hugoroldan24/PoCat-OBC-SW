# Target system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

# Toolchain prefix
set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size)

# CPU architecture flags for STM32MP157F (Cortex-M4F, FPU)
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# C flags
set(CMAKE_C_FLAGS
    "${CPU_FLAGS} -fdata-sections -ffunction-sections"
    CACHE STRING "C flags"
)

# C++ flags (for Radiolib)
set(CMAKE_CXX_FLAGS
    "${CPU_FLAGS} -fdata-sections -ffunction-sections -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-unwind-tables -fno-asynchronous-unwind-tables -std=gnu++20"
    CACHE STRING "C++ flags"
)

# ASM
set(CMAKE_ASM_FLAGS
    "${CPU_FLAGS} -fdata-sections -ffunction-sections"
    CACHE STRING "ASM flags"
)

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs -Wl,--print-memory-usage"
    CACHE STRING "Linker flags"
)

# Output file suffix
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# For cross compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

