#ifndef INSIDE_TERMOMETER_h
#define INSIDE_TERMOMETER_h

//Known bug with GPIO16 on ESP8266 + 1wire lib:
//https://github.com/PaulStoffregen/OneWire/issues/27
//works only with: 2.3.0


#define ONE_WIRE_PIN 16
#define INSIDE_T_ERROR_VALUE 0xFFFFFFFF

void insideTermometerInit();

void insideTermometerRequest();

bool insideTermometerHasValue();

float insideTermometerValue();

#endif
