#ifndef __TEMPERATURE_SENSOR_H__
#define __TEMPERATURE_SENSOR_H__

#include "mbed.h"
#include "DS1820/DS1820.h"
#include "Sensor.h"

class DS18B20_TemperatureSensor : public Sensor{

 public:
  DS18B20_TemperatureSensor(PinName pin);
  ~DS18B20_TemperatureSensor();
  float read();

 private:
  PinName assignedPin;
  DS1820 probe;

};

#endif
