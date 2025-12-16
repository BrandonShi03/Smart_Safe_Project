// gyro_motion.c - LSM6DSOX-based motion detection for tamper sensing

#include "drivers_hw123.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <board_pins.h>



// -----------------------------------------------------------------------------
// LSM6DSOX register map (subset)
// Datasheet: WHO_AM_I=0x6C at register 0x0F. :contentReference[oaicite:1]{index=1}
// Gyro outputs: OUTX_L_G (0x22) .. OUTZ_H_G (0x27) :contentReference[oaicite:2]{index=2}
// Accel outputs: OUTX_L_A (0x28) .. OUTZ_H_A (0x2D) :contentReference[oaicite:3]{index=3}
// -----------------------------------------------------------------------------
#define REG_FUNC_CFG_ACCESS  0x01
#define REG_WHO_AM_I         0x0F
#define REG_CTRL1_XL         0x10
#define REG_CTRL2_G          0x11
#define REG_CTRL3_C          0x12
#define REG_CTRL4_C          0x13
#define REG_CTRL5_C          0x14

#define REG_OUTX_L_G         0x22
#define REG_OUTX_H_G         0x23
#define REG_OUTY_L_G         0x24
#define REG_OUTY_H_G         0x25
#define REG_OUTZ_L_G         0x26
#define REG_OUTZ_H_G         0x27

#define REG_OUTX_L_A         0x28
#define REG_OUTX_H_A         0x29
#define REG_OUTY_L_A         0x2A
#define REG_OUTY_H_A         0x2B
#define REG_OUTZ_L_A         0x2C
#define REG_OUTZ_H_A         0x2D

// Expected WHO_AM_I value for LSM6DSOX. :contentReference[oaicite:4]{index=4}
#define LSM6DSOX_WHO_AM_I    0x6C

// CTRL3_C bits: BOOT BDU H_LACTIVE PP_OD SIM IF_INC 0 SW_RESET
// We want BDU=1 (bit6), IF_INC=1 (bit2). :contentReference[oaicite:5]{index=5}
#define CTRL3_C_BDU_IFINC    0x44

// Simple motion detection config
#define CALIBRATION_SAMPLES      100
#define CALIBRATION_DELAY_MS     10

// Threshold in raw accel counts for "movement". LSM6DSOX @±2g is ~16384 counts/g.
// 0.1 g ~ 1600 counts -> square ~ 2.5e6. We use squared threshold to avoid sqrt.
#define MOTION_THRESHOLD_COUNTS  1600
#define MOTION_THRESHOLD_SQ      ((int32_t)MOTION_THRESHOLD_COUNTS * (int32_t)MOTION_THRESHOLD_COUNTS)

// -----------------------------------------------------------------------------
// Internal state
// -----------------------------------------------------------------------------
static bool s_initialized    = false;
static bool s_ever_moved     = false;
static uint8_t s_motion_cnt  = 0;

// Baseline accel vector (at rest), in raw counts
static int32_t s_base_ax = 0;
static int32_t s_base_ay = 0;
static int32_t s_base_az = 0;

// -----------------------------------------------------------------------------
// Low-level I2C helpers
// -----------------------------------------------------------------------------
static bool lsm6_write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = { reg, value };
    int res = i2c_write_blocking(GYRO_I2C_INSTANCE, LSM6DSOX_I2C_ADDR, buf, 2, false);
    if (res != 2) {
        printf("LSM6DSOX: write reg 0x%02X failed, res=%d\n", reg, res);
        return false;
    }
    return true;
}

static bool lsm6_read_reg(uint8_t reg, uint8_t *value) {
    int res = i2c_write_blocking(GYRO_I2C_INSTANCE, LSM6DSOX_I2C_ADDR, &reg, 1, true);
    if (res != 1) {
        printf("LSM6DSOX: set reg 0x%02X for read failed, res=%d\n", reg, res);
        return false;
    }
    res = i2c_read_blocking(GYRO_I2C_INSTANCE, LSM6DSOX_I2C_ADDR, value, 1, false);
    if (res != 1) {
        printf("LSM6DSOX: read reg 0x%02X failed, res=%d\n", reg, res);
        return false;
    }
    return true;
}

static bool lsm6_read_multi(uint8_t start_reg, uint8_t *buf, size_t len) {
    int res = i2c_write_blocking(GYRO_I2C_INSTANCE, LSM6DSOX_I2C_ADDR, &start_reg, 1, true);
    if (res != 1) {
        printf("LSM6DSOX: set start reg 0x%02X failed, res=%d\n", start_reg, res);
        return false;
    }
    res = i2c_read_blocking(GYRO_I2C_INSTANCE, LSM6DSOX_I2C_ADDR, buf, len, false);
    if (res != (int)len) {
        printf("LSM6DSOX: read multi from 0x%02X failed, res=%d\n", start_reg, res);
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Sensor reading
// -----------------------------------------------------------------------------
static bool lsm6_read_accel_raw(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];
    if (!lsm6_read_multi(REG_OUTX_L_A, data, 6)) {
        return false;
    }

    *ax = (int16_t)((data[1] << 8) | data[0]);
    *ay = (int16_t)((data[3] << 8) | data[2]);
    *az = (int16_t)((data[5] << 8) | data[4]);
    return true;
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
bool gyro_motion_init(void) {
    printf("gyro_motion_init (LSM6DSOX): starting\n");

    // Configure I2C
    i2c_init(GYRO_I2C_INSTANCE, 400 * 1000); // 400 kHz
    gpio_set_function(GYRO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(GYRO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(GYRO_I2C_SDA_PIN);
    gpio_pull_up(GYRO_I2C_SCL_PIN);

    sleep_ms(50);

    // Check WHO_AM_I
    uint8_t who = 0;
    if (!lsm6_read_reg(REG_WHO_AM_I, &who)) {
        printf("gyro_motion_init: failed to read WHO_AM_I\n");
        return false;
    }

    printf("LSM6DSOX WHO_AM_I = 0x%02X\n", who);
    if (who != LSM6DSOX_WHO_AM_I) {
        printf("gyro_motion_init: unexpected WHO_AM_I (expected 0x%02X)\n",
               LSM6DSOX_WHO_AM_I);
        return false;
    }

    // Enable IF_INC and BDU so multi-read works nicely.
    if (!lsm6_write_reg(REG_CTRL3_C, CTRL3_C_BDU_IFINC)) {
        printf("gyro_motion_init: failed to write CTRL3_C\n");
        return false;
    }

    // Configure accelerometer: CTRL1_XL
    // Example config: 0x40 = ODR=104 Hz, ±2g, no LPF. :contentReference[oaicite:6]{index=6}
    if (!lsm6_write_reg(REG_CTRL1_XL, 0x40)) {
        printf("gyro_motion_init: failed to write CTRL1_XL\n");
        return false;
    }

    // Configure gyroscope: CTRL2_G
    // Example config: 0x40 = ODR=104 Hz, 250 dps.
    if (!lsm6_write_reg(REG_CTRL2_G, 0x40)) {
        printf("gyro_motion_init: failed to write CTRL2_G\n");
        return false;
    }

    // Small delay to let sensor settle
    sleep_ms(100);

    // Calibrate baseline accel at rest
    int64_t sum_x = 0, sum_y = 0, sum_z = 0;
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        int16_t ax, ay, az;
        if (lsm6_read_accel_raw(&ax, &ay, &az)) {
            sum_x += ax;
            sum_y += ay;
            sum_z += az;
        }
        sleep_ms(CALIBRATION_DELAY_MS);
    }

    s_base_ax = (int32_t)(sum_x / CALIBRATION_SAMPLES);
    s_base_ay = (int32_t)(sum_y / CALIBRATION_SAMPLES);
    s_base_az = (int32_t)(sum_z / CALIBRATION_SAMPLES);

    printf("LSM6DSOX baseline accel: ax=%ld ay=%ld az=%ld\n",
           (long)s_base_ax, (long)s_base_ay, (long)s_base_az);

    s_initialized   = true;
    s_ever_moved    = false;
    s_motion_cnt    = 0;

    printf("gyro_motion_init (LSM6DSOX): OK\n");
    return true;
}

bool gyro_motion_has_moved(void) {
    if (!s_initialized) {
        return false;
    }

    int16_t ax_raw, ay_raw, az_raw;
    if (!lsm6_read_accel_raw(&ax_raw, &ay_raw, &az_raw)) {
        // If read fails, don’t clear previous motion state.
        return s_ever_moved;
    }

    int32_t dx = (int32_t)ax_raw - s_base_ax;
    int32_t dy = (int32_t)ay_raw - s_base_ay;
    int32_t dz = (int32_t)az_raw - s_base_az;

    int64_t mag_sq = (int64_t)dx * dx + (int64_t)dy * dy + (int64_t)dz * dz;

    if (mag_sq > MOTION_THRESHOLD_SQ) {
        if (s_motion_cnt < 255) {
            s_motion_cnt++;
        }
    } else {
        if (s_motion_cnt > 0) {
            s_motion_cnt--;
        }
    }

    // Require a few consecutive "above threshold" samples before latching movement.
    if ( s_motion_cnt >= 3 ) {
        s_ever_moved = true;
    }

    return s_ever_moved;
}
