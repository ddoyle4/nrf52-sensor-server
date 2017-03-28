#ifndef __DEBUG_SENSOR_H__
#define __DEBUG_SENSOR_H__

#include "mbed.h"
#include "Sensor.h"

class DebugSensor : public Sensor{

 public:
  DebugSensor();
  ~DebugSensor();
  float read();

 private:
  float incrementValue;
};

#endif
