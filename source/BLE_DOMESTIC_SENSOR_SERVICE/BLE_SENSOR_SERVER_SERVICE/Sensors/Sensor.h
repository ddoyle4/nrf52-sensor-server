#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "mbed.h"

class Sensor {
 public:
  Sensor();
  ~Sensor();
  virtual float read()=0;
  float getLastReading();

 protected:
  float lastReading;
};

#endif
