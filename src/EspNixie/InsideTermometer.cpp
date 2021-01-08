#include "InsideTermometer.h"

#include <limits.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature DS18B20(&oneWire);

float value = INSIDE_T_ERROR_VALUE;
uint32_t start_conv_t;

void insideTermometerInit(){
  DS18B20.begin();
  DS18B20.setResolution(12);
}

void insideTermometerRequest(){
  start_conv_t = millis();
  DS18B20.requestTemperatures(); 
}

bool insideTermometerHasValue(){
  if (start_conv_t){
    if (millis() - start_conv_t > 1000){
      start_conv_t = 0;
      value = DS18B20.getTempCByIndex(0);
    }
  }
  
	return value != INSIDE_T_ERROR_VALUE;
}

float insideTermometerValue(){
  return value;
}
