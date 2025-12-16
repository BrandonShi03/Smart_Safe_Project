# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
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
set(sdkVersion 2.2.0)
set(toolchainVersion RISCV_ZCB_RPI_2_2_0_3)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type") #change to pico_h for testing with other board otherwise pico2_w


cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(Smart_Safe_Final_Project)
pico_sdk_init()

add_executable(Smart_Safe_Final_Project
    src/main.c
    #src/scheduler.c src/log.c src/power.c
    #src/drivers_lcd_i2c.c src/drivers_ds3231.c
    #src/drivers_mpu6050.c src/drivers_sdcard.c
	#src/drivers_buzzer.c
    src/TFTMaster.c
src/timer.c
src/TouchScreen.c
src/ts_lcd.c
src/glcdfont.c
src/sw_in.c
src/led_out.c
src/fsm.c
src/drivers_keypad_pcf8574.c
src/drivers_solenoid.c 
src/drivers_battery.c
src/drivers_reed.c
src/hc05.c
src/drivers_hw123.c
src/tamper.c
src/drivers_buzzer.c
)
pico_generate_pio_header(Smart_Safe_Final_Project ${CMAKE_CURRENT_LIST_DIR}/SPIPIO.pio)

target_include_directories(Smart_Safe_Final_Project PUBLIC include)

target_link_libraries(Smart_Safe_Final_Project
    pico_stdlib
    hardware_gpio
    hardware_i2c
hardware_uart
    hardware_pwm
    hardware_i2c
    hardware_spi
    hardware_adc
    #hardware_rtc
    pico_unique_id
    hardware_pio 
    hardware_irq
    hardware_adc
pico_bootsel_via_double_reset
# ^^ comment out if testing on pico H board
)

pico_add_extra_outputs(Smart_Safe_Final_Project)
pico_enable_stdio_usb(Smart_Safe_Final_Project 1)
pico_enable_stdio_uart(Smart_Safe_Final_Project 0)
