#ifndef WIFI_RAK410_H
#define WIFI_RAK410_H
#include "arduino.h"

#define RESET_PIN 7 //Wifi module reset pin

uint16_t DrvUART_Read(uint8_t *pu8TxBuf);
void Reset_Target(void);
uint8_t Wifi_uart_send_data(uint8_t flag,uint8_t *pu8TxBuf,uint16_t data_length);
uint8_t Get_asc_length(uint16_t data);
uint8_t Wifi_init(void);
#endif
