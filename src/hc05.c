#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "board_pins.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0  // GP0 -> HC-05 RX
#define UART_RX_PIN 1  // GP1 <- HC-05 TX
#define BUFFER_SIZE 256

//variables
char rx_buffer[BUFFER_SIZE];
int rx_index = 0;
bool new_command = false;
bool signal_received = false;
char received_command[100] = "";

//uart init
void uart_init_hc05(void) {
    uart_init(UART_ID, BAUD_RATE);
    
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);
    
}

void send_status(const char* message) {
    char buffer[BUFFER_SIZE];
    
    snprintf(buffer, sizeof(buffer), "Safe:%s\n", message);
    
    uart_puts(UART_ID, buffer);

}

// Process received command
static void process_command(const char* command) {
    printf("BT Received: %s\n", command);
    
    // Check for trigger command
    if (strcmp(command, "TRIGGER") == 0) {
        signal_received = true;
        strcpy(received_command, "TRIGGER");
        printf("Trigger signal received!\n");
        
        // Send acknowledgment
        send_status("TRIGGER_RECEIVED");
    }
    else {
        printf("Unknown command: %s\n", command);
        send_status("ERROR:UNKNOWN_COMMAND");
    }
}

void bluetooth_check_rx(void) {
    while (uart_is_readable(UART_ID)) {
        char c = uart_getc(UART_ID);
        signal_received = true;
        // Debug: print raw character (optional)
        // printf("BT Raw: %c (0x%02X)\n", c, c);
        
        // Check for end of command (newline or A)
        if (c == '\n' || c == 'A') {
            if (rx_index > 0) {
                // Null-terminate the string
                rx_buffer[rx_index] = '\0';
                signal_received = true;
                // Process the command
                process_command(rx_buffer);
                
                // Reset buffer index
                rx_index = 0;
            }
        }
        // Check for buffer overflow
        else if (rx_index < BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = c;
        }
        else {
            // Buffer overflow - reset and send error
            printf("Bluetooth buffer overflow!\n");
            send_status("ERROR:BUFFER_OVERFLOW");
            rx_index = 0;
        }
    }
}

// Clear the signal flag (to be called from FSM after processing)
void bluetooth_clear_signal(void) {
    signal_received = false;
    received_command[0] = '\0';
    printf("Bluetooth signal cleared\n");
}

// Check if trigger signal was received
bool bluetooth_is_trigger_received(void) {
    return signal_received;
}
