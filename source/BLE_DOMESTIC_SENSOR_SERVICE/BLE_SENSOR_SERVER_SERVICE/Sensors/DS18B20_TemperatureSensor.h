#ifndef __TEMPERATURE_SENSOR_H__
#define __TEMPERATURE_SENSOR_H__

#include "mbed.h"
#include "DS1820/DS1820.h"

class DS18B20_TemperatureSensor {

 public:
  DS18B20_TemperatureSensor(PinName pin);
  ~DS18B20_TemperatureSensor();
  float read();

 private:
  PinName assignedPin;
  DS1820 probe;

};

#endif
