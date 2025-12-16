#include "drivers_battery.h"
#include "drivers_battery.h"
#include "board_pins.h"
#include "hardware/adc.h"

//Measured Resistor Values
static const float R_TOP = 19.70f;
static const float R_BOT = 9.87f;

//Scale factor using resistor values
static const float SCALE = (R_TOP + R_BOT) / R_BOT; // ~ 2.996

//Countdown variables
uint16_t cooldown_counter = 0;
uint16_t threshold = 5;

void battery_init(void) {
    adc_init();

    //Enable adc function on gpio defined in board_pins.h
    adc_gpio_init(BATT);

    //Select adc channel defined in board_pins.h
    adc_select_input(ADC_CH_BATTERY);
}

float battery_read_voltage(void){
    const float VREF = 3.3f;  //ADC Reference

    uint16_t raw = adc_read();

    float v_adc = (raw * VREF) / 4095.0f;
    float v_batt = v_adc * SCALE;

    return v_batt;
}

//Very rough % estimate;  Tweak V_MAX as needed
float battery_read_percentage(void){
        float v = battery_read_voltage();

        float V_MIN = 4.0f; //empty
        float V_MAX = 5.2f; //Full

        float p = (v-V_MIN) / (V_MAX - V_MIN) * 100.0f;

        if(p < 0.0f) p = 0.0f;
        if (p > 100.0f) p = 100.0f;

        return p;
}