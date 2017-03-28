#include "DebugSensor.h"


DebugSensor::DebugSensor() :
  Sensor(),
  incrementValue(0.0)
{}

DebugSensor::~DebugSensor()
{}

float DebugSensor::read(){
  incrementValue += 1.0;
  
  return incrementValue;
}

