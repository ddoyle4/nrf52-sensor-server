#include "Sensor.h"

Sensor::Sensor() :
  lastReading(0)
{}

Sensor::~Sensor()
{}

float Sensor::getLastReading(){ return lastReading; }
