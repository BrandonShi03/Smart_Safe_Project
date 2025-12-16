#include "drivers_keypad_pcf8574.h"
#include <stdio.h>
#include "board_pins.h"

const char KEYPAD_MAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'}, 
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


const uint row_pins[KEYPAD_ROWS] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};

 
const uint col_pins[KEYPAD_COLS] = {COL1_PIN, COL2_PIN, COL3_PIN, COL4_PIN};

void keypad_init(void) {
    printf("Initializing Keypad...\n");

    for (int i = 0; i < KEYPAD_ROWS; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], 1); 
    }
    
    for (int i = 0; i < KEYPAD_COLS; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_up(col_pins[i]);
    }
    
    printf("Keypad initialized successfully\n");
}

char keypad_get_key(void) {
    for (int row = 0; row < KEYPAD_ROWS; row++) {
        gpio_put(row_pins[row], 0);
        
        sleep_us(20);
        
        for (int col = 0; col < KEYPAD_COLS; col++) {
            if (gpio_get(col_pins[col]) == 0) {
                sleep_ms(20); 
                
                if (gpio_get(col_pins[col]) == 0) {
                    while (gpio_get(col_pins[col]) == 0) {
                        sleep_ms(10);
                    }
                    
                    gpio_put(row_pins[row], 1);
                    
                    return KEYPAD_MAP[row][col];
                }
            }
        }
        
        gpio_put(row_pins[row], 1);
    }
    
    return '\0';
}

char keypad_wait_for_key(void) {
    char key;
    do {
        key = keypad_get_key();
        sleep_ms(50); 
    } while (key == '\0');
    
    return key;
}

bool keypad_is_key_pressed(char key) {
    char current_key = keypad_get_key();
    return (current_key == key);
}
