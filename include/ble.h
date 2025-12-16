/**
 * Bluetooth Low Energy header file for smart safe final project
 */
#ifndef BLE_H
#define BLE_H

#include <stdint.h>
#include <stdbool.h>
#include "btstack.h"

// Function declarations
void bluetooth_init(void);
void send_string_to_phone(const char* message);

#endif // BLE_H