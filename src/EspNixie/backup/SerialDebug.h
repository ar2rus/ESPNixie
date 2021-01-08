#ifndef SERIAL_DEBUG_h
#define SERIAL_DEBUG_h

#include <ESP8266WiFi.h>

#define DEBUG 0

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 5

void serailDebug_init();
void serailDebug_update();

void _print(String text);
void _println(String text);
void _print(int v);
void _println(int v);

void _print(char v);
void _println(char v);

#endif
