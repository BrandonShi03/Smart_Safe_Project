#ifndef TS_LCD_H
#define TS_LCD_H
#include "pico/stdlib.h"
#include "stdbool.h"
#include "inttypes.h"

bool get_ts_lcd(uint16_t *px, uint16_t *py);

void ts_lcd_init();

#endif