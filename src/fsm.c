//This is FSM function file for smart safe final project for ece_414 @Lafayette college
//Author:Jackson Kaminski,Haoxuan Shi
//Date: Fall 2025

#include "fsm.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>
#include <drivers_solenoid.h>
//testing 
#include <stdint.h>
//LCD Display Library
#include "ts_lcd.h" 
#include "TFTMaster.h"

//keypad
#include "drivers_keypad_pcf8574.h"

//reed
#include "drivers_reed.h"

//bluetooth
#include "hc05.h"

//tamper detection
#include "tamper.h"

//buzzer
#include "drivers_buzzer.h"


//keypad init
static char entered_pin[MAX_PIN_LENGTH + 1] = "";
static int pin_length = 0; //current pin length
static const char* correct_pin = "1234"; // correct pin

//variables
#define PASSWORD_LENGTH 4
#define UPDATE_INTERVAL_MS 120000  // 120,000ms = 2min
char current_pass[5] = "1234";  // current password
uint32_t last_update_time = 115000;   // last update time

enum SAFE_STATES{
    STATE_LOCKED, //locked safe
    STATE_PASSWORD,    //Enter password for dual auth
    STATE_BLUETOOTH,//using bluetooth as 2fa
    STATE_UNLOCKED, // safe unlocked
    STATE_OPEN, //opens safe
    STATE_TAMPER
}Safe_State;

void fsm_init(void){
    Safe_State = STATE_LOCKED;
}

void generate_new_password(char *buffer) {
    static uint32_t counter = 0;
    counter++;
    
    // simple random password generator
    uint32_t time_ms = to_ms_since_boot(get_absolute_time());
    uint32_t n = (counter * time_ms) % 10000;  // 0-9999
    
    buffer[0] = '0' + (n / 1000) % 10;
    buffer[1] = '0' + (n / 100) % 10;
    buffer[2] = '0' + (n / 10) % 10;
    buffer[3] = '0' + n % 10;
    buffer[4] = '\0';
}

void show_next_update_time(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // calculate time left
    uint32_t time_since_last = now - last_update_time;
    uint32_t time_until_next;
    
    // added time arrived
    if (time_since_last >= UPDATE_INTERVAL_MS) {
        time_until_next = 0; // update
    } else {
        time_until_next = UPDATE_INTERVAL_MS - time_since_last;
    }
    
    uint32_t minutes = time_until_next / 60000;
    uint32_t seconds = (time_until_next % 60000) / 1000;
    
    tft_setTextSize(2);
    tft_setTextColor(ILI9340_WHITE);
    tft_setCursor(20,200);
    
    char time_str[30];
    
    // display time by case
    if (time_until_next == 0) {
        snprintf(time_str, sizeof(time_str), "Updating!");
    } else if (time_until_next < 10000) { //last 10s
        snprintf(time_str, sizeof(time_str), "Update:%02lus!", seconds);
    } else {
        snprintf(time_str, sizeof(time_str), "Update:%02lu:%02lu", minutes, seconds);
    }
    
    tft_writeString(time_str);
}

void check_and_update_password(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    // check if over 2min
    if (now - last_update_time >= UPDATE_INTERVAL_MS) {
        // newcode
        generate_new_password(current_pass);
        last_update_time = now;
        show_next_update_time();
    }
}

void fsm_tick(){
    if(tamper_detected()){
        if (Safe_State != STATE_OPEN && Safe_State != STATE_UNLOCKED){
            Safe_State = STATE_TAMPER;
        }
    }

    switch (Safe_State){
    case STATE_LOCKED:
        solenoid_engage_hold();
        buzzer_off();

        tft_setTextSize(2);
        tft_setTextColor(ILI9340_WHITE);
        tft_setCursor(100,100);
        tft_writeString("LOCKED");
        tft_setCursor(100,130);
        tft_writeString("PRESS A");

        char lockkey = keypad_get_key();
        if (lockkey == 'A') {
            Safe_State = STATE_PASSWORD;
            //Safe_State = STATE_UNLOCKED;
            send_status("Authing");
        } 
    break;

    case STATE_PASSWORD:
        //Safe_State = STATE_LOCKED;
        tft_setTextSize(2);
        tft_setCursor(100,100);
        tft_setTextColor(ILI9340_WHITE);
        //tft_writeString("Waiting for password");
        char display_pin[MAX_PIN_LENGTH +1] = "";
        for (int i=0; i<MAX_PIN_LENGTH ; i++){    
            if (i < pin_length) {
                        char temp[2] = {entered_pin[i], '\0'};
                        strcat(display_pin, temp);
                } 
            else {
                strcat(display_pin, "_");
            }
        }
        tft_writeString(display_pin);

        //pin input tips
        tft_setCursor(100,130);
        tft_writeString("#=TRY *=clr");

        char key = keypad_get_key();
        if (key != '\0') {
            if (key >= '0' && key <= '9') {
                if (pin_length < MAX_PIN_LENGTH) {
                    entered_pin[pin_length] = key;
                    pin_length++;
                    entered_pin[pin_length] = '\0';

                    tft_fillRect(100, 120, 200, 30, ILI9340_BLUE);
                    tft_setCursor(100, 100);

                    memset(display_pin, 0, sizeof(display_pin));
                    for (int i = 0; i < MAX_PIN_LENGTH; i++) {
                        if (i < pin_length) {
                            char temp[2] = {entered_pin[i], '\0'};
                            strcat(display_pin, temp);
                        } else {
                            strcat(display_pin, "_");
                        }
                    }
                    tft_writeString(display_pin);
                }
            }
            else if (key == '*') {
                // clr
                pin_length = 0;
                memset(entered_pin, 0, sizeof(entered_pin));
            } 
            else if (key == '#') {
                // enter
                if (strcmp(entered_pin, correct_pin) == 0) {
                    tft_setCursor(80, 150);
                    tft_setTextColor(ILI9340_GREEN);
                    tft_setTextSize(2);
                    tft_writeString("CORRECT PIN");
                    
                    Safe_State = STATE_BLUETOOTH;
                    send_status("BlueTooth");

                    // reset pin
                    pin_length = 0;
                    memset(entered_pin, 0, sizeof(entered_pin));
                    
                    //force update new pin
                    generate_new_password(current_pass);
                    last_update_time = to_ms_since_boot(get_absolute_time());
                    sleep_ms(2000);
                    show_next_update_time();
                    
                } else {
                    tft_setCursor(80, 150);
                    tft_setTextColor(ILI9340_RED);
                    tft_setTextSize(2);
                    tft_writeString("WRONG PIN");
                    sleep_ms(2000);

                    // reset pin
                    pin_length = 0;
                    memset(entered_pin, 0, sizeof(entered_pin));
                    Safe_State = STATE_LOCKED;
                }
            }
        }
    break;

    case STATE_BLUETOOTH:
        tft_setTextSize(2);
        tft_setCursor(100,100);
        tft_setTextColor(ILI9340_WHITE);
        char display_bu_pin[MAX_PIN_LENGTH +1] = "";

        // shows every sec
        static uint32_t last_display = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        if (now - last_display >= 1000) {  // display every sec
            show_next_update_time();
            check_and_update_password();
            last_display = now;
            
            //send password to phone
            char message[50];
            snprintf(message,sizeof(message),"Password:%s",current_pass);
            send_status(message);
        }

        for (int i=0; i<MAX_PIN_LENGTH ; i++){    
            if (i < pin_length) {
                        char temp[2] = {entered_pin[i], '\0'};
                        strcat(display_bu_pin, temp);
                } 
            else {
                strcat(display_bu_pin, "_");
            }
        }
        //display pin
        tft_setCursor(100,100);
        tft_writeString(display_bu_pin);

        //display pin input tips
        tft_setCursor(100,130);
        tft_writeString("#=TRY *=clr");

        char bu_key = keypad_get_key();
        if (bu_key != '\0') {
            if (bu_key >= '0' && bu_key <= '9') {
                if (pin_length < MAX_PIN_LENGTH) {
                    entered_pin[pin_length] = bu_key;
                    pin_length++;
                    entered_pin[pin_length] = '\0';

                    tft_fillRect(100, 120, 200, 30, ILI9340_BLUE);
                    tft_setCursor(100, 100);

                    memset(display_bu_pin, 0, sizeof(display_bu_pin));
                    for (int i = 0; i < MAX_PIN_LENGTH; i++) {
                        if (i < pin_length) {
                            char temp[2] = {entered_pin[i], '\0'};
                            strcat(display_bu_pin, temp);
                        } else {
                            strcat(display_bu_pin, "_");
                        }
                    }
                    tft_writeString(display_bu_pin);
                }
            }
            else if (bu_key == '*') {
                // clr
                pin_length = 0;
                memset(entered_pin, 0, sizeof(entered_pin));
            } 
            else if (bu_key == '#') {
                // enter
                if (strcmp(entered_pin, current_pass) == 0) {
                    tft_setCursor(80, 150);
                    tft_setTextColor(ILI9340_GREEN);
                    tft_setTextSize(2);
                    tft_writeString("CORRECT PIN");
                    //change state and notify phone
                    Safe_State = STATE_UNLOCKED;
                    send_status("Safe:Open");
                    // reset pin
                    pin_length = 0;
                    memset(entered_pin, 0, sizeof(entered_pin));
                    sleep_ms(2000);
                } else {
                    tft_setCursor(80, 150);
                    tft_setTextColor(ILI9340_RED);
                    tft_setTextSize(2);
                    tft_writeString("WRONG PIN");
                    sleep_ms(2000);
                    // reset pin
                    pin_length = 0;
                    memset(entered_pin, 0, sizeof(entered_pin));
                    Safe_State = STATE_LOCKED;
                }
            }
        }
    break;

    case STATE_UNLOCKED:
        solenoid_disengage();
        tft_setTextSize(2);
        tft_setCursor(100,100);
        tft_setTextColor(ILI9340_WHITE);
        tft_writeString("UNLOCKED");
        sleep_ms(1500);
        Safe_State = STATE_OPEN;
    break;

    case STATE_OPEN:
        tft_setTextSize(2);
        tft_setCursor(100,100);
        tft_setTextColor(ILI9340_WHITE);
        tft_writeString("Press A to Lock");
        if (keypad_get_key() == 'A') {
            Safe_State = STATE_LOCKED;
            send_status("Closed by Key");
        }
    break;

    case STATE_TAMPER:
        tft_fillScreen(ILI9340_BLACK);
        tft_setTextSize(2);
        tft_setTextColor(ILI9340_RED);
        tft_setCursor(40, 100);
        tft_writeString("TAMPER DETECTED!");

        buzzer_on();

        //notify over Bluetooth
        send_status("ALERT: Tamper detected!");

        //allow manual reset with a special key, e.g. 'B'
        if (keypad_get_key() == 'A') {
            // Clear screen, go back to locked state
            tft_fillScreen(ILI9340_BLACK);
            Safe_State = STATE_LOCKED;
            send_status("Tamper cleared, locked");
        }

    break;
    }
}
