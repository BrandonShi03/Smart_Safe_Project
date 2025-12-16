#ifndef KEYPAD_H
#define KEYPAD_H

#include "pico/stdlib.h"

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

extern const char KEYPAD_MAP[KEYPAD_ROWS][KEYPAD_COLS];

void keypad_init(void);
char keypad_get_key(void);
char keypad_wait_for_key(void);
bool keypad_is_key_pressed(char key);

#endif
