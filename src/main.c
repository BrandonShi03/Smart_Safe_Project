//This is main file for smart safe final project for ece_414 @Lafayette college
//Author:Jackson Kaminski,Haoxuan Shi
//Date: Fall 2025

//pins connection defined
#include "board_pins.h"
//general library
#include "pico/stdlib.h" 

#include "hardware/gpio.h"
#include "hardware/adc.h"

//Reed
#include "drivers_reed.h"

//FSM
#include "fsm.h"

//RFID
#include "drivers_rc522.h"
#include "rfid-spi.h"

//battery
#include "drivers_battery.h"

//solenoid
#include "drivers_solenoid.h"

//keypad
#include "drivers_keypad_pcf8574.h"

//bluetooth module
#include "hc05.h"

//LCD Display Library
#include "ts_lcd.h" 
#include "TFTMaster.h"

//testing buttons
#include "sw_in.h"
#include "led_out.h"

//general library
#include <stdio.h>
#include <stdint.h>
#include "stdbool.h"
#include "inttypes.h"
#include "hardware/i2c.h"


//buzzer
#include "drivers_buzzer.h"

//Gyroscope
#include "drivers_hw123.h"

#include "tamper.h"

int main(){
    //init hardwares
    stdio_init_all();
    sleep_ms(2000);
    ts_lcd_init();
    sw_in_init();
    led_out_init();
    fsm_init();
    uart_init_hc05();

    bool switch1_state, switch2_state;
    

    //solenoid init
    solenoid_init(SOLENOID_GPIO);
    solenoid_set_default_pulse_ms(300);
    solenoid_engage(0);

    //reed switch init
    reed_init();

    //Battery percentage init
    battery_init();

    //buuzer
    buzzer_init();


    //keypad init
    keypad_init();
    
    tft_fillScreen(ILI9340_BLUE);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    //while(1){
        printf("%c\n", keypad_get_key());
    //}

    //Gyroscope
    if (!gyro_motion_init()) {
        while (1) {
            printf("Main: gyro_motion_init FAILED, halting.\n");
            tft_fillRect(0, 40, 300, 200, ILI9340_BLUE);
            tft_setTextSize(2);
            tft_setCursor(50, 20);
            tft_setTextColor(ILI9340_RED);
            tft_writeString("Gyroscope Init Failed!");
            sleep_ms(1000);
        }
    }


    while(1) {
        //solenoid
        solenoid_update();

        //password detects
        //check_and_update_password();

        //LCD update parts
        tft_fillRect(0, 40, 300, 200, ILI9340_BLUE);
        tft_setTextSize(2);
        fsm_tick();
        tft_setCursor(50, 20);
        tft_setTextColor(ILI9340_WHITE);
        tft_writeString("Smart Safe System");

        printf("%d\n",tamper_detected());

        //check for bluetooth commands
        bluetooth_check_rx();

        //to be changed to other counting function in the future
        sleep_ms(200);
    }
}