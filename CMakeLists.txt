cmake_minimum_required(VERSION 3.4)

project(nucleo-sd)

set(MBED_PATH "/opt/ARM/mbed")
set(MBED_LIB_PATH "${MBED_PATH}/libraries/mbed")

# collect nucleo source files
file(GLOB mbed_src "${MBED_LIB_PATH}/common/*.c" "${MBED_LIB_PATH}/common/*.cpp")
file(GLOB mbed_cmsis_src "${MBED_LIB_PATH}/targets/cmsis/TARGET_STM/TARGET_STM32F1/*.c" "${MBED_LIB_PATH}/targets/cmsis/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/*.c")
file(GLOB mbed_hal_src "${MBED_LIB_PATH}/targets/hal/TARGET_STM/TARGET_STM32F1/*.c" "${MBED_LIB_PATH}/targets/hal/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/*.c")
file(GLOB fs_sd_src "${MBED_PATH}/libraries/fs/sd/*.c" "${MBED_PATH}/libraries/fs/sd/*.cpp")
file(GLOB fs_fat_src "${MBED_PATH}/libraries/fs/fat/*.cpp" "${MBED_PATH}/libraries/fs/fat/ChaN/*.cpp")

# collect startup file
set(mbed_startup_src "/opt/ARM/mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/TOOLCHAIN_GCC_ARM/startup_stm32f103xb.S")
set_property(SOURCE ${mbed_startup_src} PROPERTY LANGUAGE C)

# collect the linker script
set(LINKER_SCRIPT "/opt/ARM/mbed/libraries/mbed/targets/cmsis/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/TOOLCHAIN_GCC_ARM/STM32F103XB.ld")

# add the nucleo files into the header path
include_directories("${MBED_LIB_PATH}/api")
include_directories("${MBED_LIB_PATH}/hal")
include_directories("${MBED_LIB_PATH}/targets/cmsis")
include_directories("${MBED_LIB_PATH}/targets/cmsis/TARGET_STM/TARGET_STM32F1")
include_directories("${MBED_LIB_PATH}/targets/cmsis/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB")
include_directories("${MBED_LIB_PATH}/targets/hal/TARGET_STM/TARGET_STM32F1")
include_directories("${MBED_LIB_PATH}/targets/hal/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB")
include_directories("${MBED_PATH}/libraries/fs/sd/")
include_directories("${MBED_PATH}/libraries/fs/fat/")
include_directories("${MBED_PATH}/libraries/fs/fat/ChaN")

# collect the application files
file(GLOB src "*.cpp" "*.c")

# choose the compiler
set(CMAKE_C_COMPILER "/usr/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/arm-none-eabi-g++")
set(CMAKE_LINKER "/usr/bin/arm-none-eabi-ld")

# set up the compiler settings
set(EXTRA_LIBS "")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT} ${EXTRA_LIBS} -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--wrap,main")


set(ARCH_OPTIONS "-mthumb -mcpu=cortex-m3")
set(CC_SYMBOLS "-DTARGET_FF_ARDUINO -DMBED_BUILD_TIMESTAMP=1456171594.73 -DTARGET_NUCLEO_F103RB -DTOOLCHAIN_GCC -DTARGE_FF_MORPHO -DTARGET_LIKE_CORTEX_M3 -DTARGET_CORTEX_M -DTARGET_LIKE_MBED -DTARGET_STM32F1 -D__MBED__=1 -DARM_MATH_CM3 -DTARGET_STM -DTOOLCHAIN_GCC_ARM -D__CORTEX_M3 -DTARGET_M3 -DTARGET_STM32F103RB")

set(CMAKE_C_FLAGS "-Wall -Wextra -ffunction-sections -fdata-sections ${ARCH_OPTIONS} ${CC_SYMBOLS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++0x -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${ARCH_OPTIONS}")

add_library(mbed STATIC ${mbed_src})
add_executable(nucleo-sd ${src} ${fs_sd_src} ${fs_fat_src} ${mbed_hal_src} ${mbed_startup_src} ${mbed_cmsis_src})
target_link_libraries(nucleo-sd mbed)

add_custom_command(TARGET nucleo-sd
  POST_BUILD
  COMMAND arm-none-eabi-objcopy -O binary nucleo-sd nucleo-sd.bin
)
