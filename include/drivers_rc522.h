//This is RC522 header file for smart safe final project for ece_414 @Lafayette college
//Author:Jackson Kaminski,Haoxuan Shi
//Date: Fall 2025

#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>

// RFID PINS
#define RFID_SPI_PORT spi_default
#define RFID_SPI_BAUDRATE (1000 * 1000)
#define RFID_CS_PIN 6
#define RFID_SCK_PIN 2  
#define RFID_MOSI_PIN 3
#define RFID_MISO_PIN 4

// RFID Structure and Type
typedef struct {
    uint8_t size;
    uint8_t uidByte[10];
    uint8_t sak;
} rfid_uid_t;

typedef enum {
    RFID_STATUS_OK,
    RFID_STATUS_ERROR,
    RFID_STATUS_NO_CARD,
    RFID_STATUS_READ_ERROR,
    RFID_STATUS_INIT_FAILED
} rfid_status_t;

typedef enum {
    RFID_CARD_UNKNOWN = 0,
    RFID_CARD_MIFARE_1K,
    RFID_CARD_MIFARE_4K,
    RFID_CARD_MIFARE_UL,
    RFID_CARD_MIFARE_MINI,
    RFID_CARD_MIFARE_PLUS,
    RFID_CARD_ISO_14443_4
} rfid_card_type_t;

// RFID functions
rfid_status_t rfid_init(void);
bool rfid_is_card_present(void);
rfid_status_t rfid_read_card(rfid_uid_t *output_uid, uint32_t *card_id, rfid_card_type_t *card_type);
const char* rfid_get_card_type_name(rfid_card_type_t card_type);
const char* rfid_get_status_message(rfid_status_t status);

#endif // RFID_DRIVER_H