#pragma once
#include <ESP8266WiFi.h>

#define ONE_SECOND_MS 1000UL
#define STATE_ROLL_TIME 10000UL
#define TEMP_REFRESH_TIME 5000UL
#define UDP_PORT_BROADCAST 29250

/* Input pins */
#define CFG_PUSH_PIN 0
#define TEMP_PIN 13

#define HW_UART_TXD 1
#define HW_UART_RXD 3

/* MQTT Client */
#define MQTT_RECONNECTION_TIME 15

#define DEVICE_NAME_STRING String("bmon")