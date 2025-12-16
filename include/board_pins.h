#ifndef BOARD_PINS_H
#define BOARD_PINS_H

//bluetooth
#define BLU_RXD 0
#define BLU_TXD 1

//gyroscope
#define GYRO_I2C_INSTANCE   i2c1
#define GYRO_I2C_SDA_PIN    6
#define GYRO_I2C_SCL_PIN    7
#define LSM6DSOX_I2C_ADDR       0x6A   // MPU6050 with AD0=0

#define BUZZER_PIN 15  // <-- EDIT to your actual buzzer GPIO pin

//Keypad
#define COL1_PIN 9
#define COL2_PIN 10
#define COL3_PIN 21
#define COL4_PIN 22
#define ROW1_PIN 11
#define ROW2_PIN 12  
#define ROW3_PIN 13
#define ROW4_PIN 14

//LCD
#define DC 16
#define SCK 17
#define CS 18
#define MOSI 19
#define RST 20

//Onboard LED
#define ONBOARD_LED 25

//Battery
#define BATT 26
#define ADC_CH_BATTERY 0 //Adc channel for gpio28 (ADC2)

//Solenoid
#define SOLENOID_GPIO 27

//Reed
#define REED_PIN 28

#endif
