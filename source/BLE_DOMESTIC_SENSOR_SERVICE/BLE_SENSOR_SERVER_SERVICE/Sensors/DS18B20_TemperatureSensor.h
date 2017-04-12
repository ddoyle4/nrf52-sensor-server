#ifndef __TEMPERATURE_SENSOR_H__
#define __TEMPERATURE_SENSOR_H__

#include "mbed.h"
#include "DS1820/DS1820.h"
#include "Sensor.h"

class DS18B20_TemperatureSensor : public Sensor{

 public:
  static const float SANITY_MAX_VALUE = 130.0;
  static const float SANITY_MIN_VALUE = -60.0;
  static const int SANITY_RETRY = 7;
  static const float SANITY_FAIL_VALUE = 333.333;
  DS18B20_TemperatureSensor(PinName pin);
  ~DS18B20_TemperatureSensor();
  float read();

 private:
  PinName assignedPin;
  DS1820 probe;

  bool isSane(float value);

};

#endif
