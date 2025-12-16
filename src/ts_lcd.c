#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "stdbool.h"
#include "inttypes.h"
#include "TouchScreen.h"
#include "TFTMaster.h"
#include "ts_lcd.h"

// defining min/max valus for touchscreen raw ADC values
#define TS_MIN_X 700
#define TS_MAX_X 3600
#define TS_MIN_Y 500
#define TS_MAX_Y 3600
// defining screen resolution, 240 x 320 pixels
#define ADAFRUIT_LCD_MAX_X 239
#define ADAFRUIT_LCD_MAX_Y 319

/* helper function to do linear interpolation, according to slide 20 lecture 10.
 * Parameters:
 *      raw : touchscreen coordinate
 *      min_raw : min raw touchscreen coordinate
 *      max_raw : max raw touchscreen coordinate
 *      min_lcd : min lcd coordinate (usually 0 in our case)
 *      max_lcd : max lcd coordinate
*/

static uint16_t linear_interpolate(uint16_t raw, uint16_t min_raw, uint16_t max_raw, uint16_t min_lcd, uint16_t max_lcd) {
    // Calculate the difference between max and min in raw touchscreen & lcd coordinate
    int32_t range_raw = (int32_t)max_raw - (int32_t)min_raw;
    int32_t range_lcd = (int32_t)max_lcd - (int32_t)min_lcd;
    // Now rescale back
    int32_t rescaled = (((int32_t)raw - (int32_t)min_raw) * range_lcd) / range_raw;
    return (uint16_t)(min_lcd + rescaled);
}

bool get_ts_lcd(uint16_t *px, uint16_t *py) {
    struct TSPoint p;
    p.x = 0;
    p.y = 0;
    p.z = 0;
    getPoint(&p);
    uint16_t lcd_x, lcd_y; // for rescaled x- and y- lcd coordinates
    *px = linear_interpolate(p.x, TS_MIN_X, TS_MAX_X, 0, ADAFRUIT_LCD_MAX_X);
    *py = linear_interpolate(p.y, TS_MIN_Y, TS_MAX_Y, 0, ADAFRUIT_LCD_MAX_Y);
    if (p.z <= 1000) { // if screen is touched, z < 1000
        return true; 
    }
    return false;
}

void ts_lcd_init() {
    adc_init();
    // initialize screen
    tft_init_hw();
    tft_begin();
    tft_setRotation(3);//was 3
    tft_fillScreen(ILI9340_BLACK);
    return;
}