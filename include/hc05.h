#ifndef HC_H
#define HC_H
void uart_init_hc05(void);
void send_status(const char* message);
void bluetooth_check_rx(void);
void bluetooth_clear_signal(void);
bool bluetooth_is_trigger_received(void);

#endif