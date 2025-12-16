#ifndef DRIVERS_HW
#define DRIVERS_HW

#include <stdbool.h>

/**
 * @brief Initialize the gyroscope + motion detection.
 *
 * - Sets up I2C for the HW123/MPU6050
 * - Wakes the chip and configures ranges
 * - Calibrates while device is still
 *
 * @return true on success, false on sensor error.
 */
bool gyro_motion_init(void);

/**
 * @brief Check if the device has EVER moved since initialization.
 *
 * Call this regularly (e.g. in your main loop).
 *
 * @return true if motion has been detected at least once since init.
 */
bool gyro_motion_has_moved(void);

#endif // GYRO_MOTION_H
