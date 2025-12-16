#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "btstack.h"

// Include the auto-generated GATT database
#include "pico_safe.gatt.h"

// Global variables
static uint16_t custom_char_value_handle = ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_VALUE_HANDLE;
static uint16_t custom_char_cccd_handle = ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_CLIENT_CONFIGURATION_HANDLE;
static int  le_notification_enabled = 0;
static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;

// Custom data
static uint8_t safe_data_value = 0;
static char safe_status_string[50];
static int safe_status_string_len = 0;

// Advertising data
static const uint8_t adv_data[] = {
    // Flags: LE General Discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Complete Local Name
    0x0C, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'i', 'c', 'o', ' ', 'S', 'a', 'f', 'e',
    // 16-bit Service UUIDs (using custom UUID from .gatt file)
    0x03, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS, 0x10, 0xFF
};

static const uint8_t adv_data_len = sizeof(adv_data);

/**
 * Send number to connected phone via BLE notification
 */
void send_string_to_phone(const char* message) {
    if (le_notification_enabled && con_handle != HCI_CON_HANDLE_INVALID) {
        // Send notification
        att_server_notify(con_handle, 
                         ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_VALUE_HANDLE, 
                         (uint8_t*) message, strlen(message));
    }
}

/**
 * Packet handler for Bluetooth events
 */
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
   
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_notification_enabled = 0;
            con_handle = HCI_CON_HANDLE_INVALID;
            printf("BLE: Device disconnected\n");
            break;
            
        case ATT_EVENT_CAN_SEND_NOW:
            // Send pending notification
            att_server_notify(con_handle, 
                             ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_VALUE_HANDLE, 
                             (uint8_t*) safe_status_string, safe_status_string_len);
            break;
            
        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                    printf("BLE: New connection, handle: 0x%04X\n", con_handle);
                    break;
            }
            break;
            
        default:
            break;
    }
}

/**
 * ATT Read Callback
 */
static uint16_t att_read_callback(hci_con_handle_t connection_handle, 
                                 uint16_t att_handle, 
                                 uint16_t offset, 
                                 uint8_t *buffer, 
                                 uint16_t buffer_size) {
    UNUSED(connection_handle);

    printf("BLE: Read request handle 0x%04X\n", att_handle);
    
    // Handle custom characteristic read
    if (att_handle == custom_char_cccd_handle) {
        return att_read_callback_handle_blob((const uint8_t *)safe_status_string, 
                                           safe_status_string_len, offset, buffer, buffer_size);
    }
    
    return 0;
}

/**
 * ATT Write Callback
 */
static int att_write_callback(hci_con_handle_t connection_handle, 
                             uint16_t att_handle, 
                             uint16_t transaction_mode, 
                             uint16_t offset, 
                             uint8_t *buffer, 
                             uint16_t buffer_size) {
    UNUSED(transaction_mode);
    UNUSED(offset);

    printf("BLE: Write request handle 0x%04X, size: %d\n", att_handle, buffer_size);
    
    switch (att_handle) {
        case ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_CLIENT_CONFIGURATION_HANDLE:
            // Handle CCCD write (enable/disable notifications)
            if (buffer_size == 2) {
                uint16_t cccd_value = little_endian_read_16(buffer, 0);
                le_notification_enabled = (cccd_value == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                con_handle = connection_handle;
                
                printf("BLE: Notifications %s\n", 
                       le_notification_enabled ? "ENABLED" : "DISABLED");
                
                if (le_notification_enabled) {
                    // Send initial status
                    send_string_to_phone("Notification Enabled");
                }
            }
            break;
            
        case ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_VALUE_HANDLE:
            // Handle write to characteristic value
            printf("BLE: Characteristic write: ");
            printf_hexdump(buffer, buffer_size);
            break;
            
        default:
            break;
    }
    
    return 0;
}


void bluetooth_init(void) {
    stdio_init_all();
    
    // Initialize Bluetooth stack
    l2cap_init();
    sm_init();
    
    // Setup ATT server with auto-generated GATT database
    att_server_init(profile_data, att_read_callback, att_write_callback);
    
    // Setup advertising
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 
                                0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_advertisements_enable(1);
    
    // Register event handlers
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(packet_handler);
    
    // Initialize safe data
    safe_data_value = 0;
    safe_status_string_len = snprintf(safe_status_string, sizeof(safe_status_string), 
                                     "Safe Status: Ready");
    
    printf("BLE: Pico Safe initialized successfully\n");
    printf("BLE: Custom Characteristic Handle: 0x%04X\n", 
           ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_VALUE_HANDLE);
    printf("BLE: CCCD Handle: 0x%04X\n", 
           ATT_CHARACTERISTIC_12345678_9ABC_DEF0_1234_56789ABCDEF2_01_CLIENT_CONFIGURATION_HANDLE);
    
    // Power on Bluetooth
    hci_power_control(HCI_POWER_ON);
    
    printf("BLE: Advertising as 'Pico Safe'\n");
}