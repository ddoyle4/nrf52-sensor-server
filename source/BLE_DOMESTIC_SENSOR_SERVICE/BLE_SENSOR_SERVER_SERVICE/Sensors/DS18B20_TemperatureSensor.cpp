#include "DS18B20_TemperatureSensor.h"


DS18B20_TemperatureSensor::DS18B20_TemperatureSensor(PinName pin) :
  Sensor(),
  assignedPin(pin),
  probe(DS1820(pin))
{
  // TODO maybe move the initialisation of the DS1820 to here and have repeated try/catch
  // block with time number of errors
  //allow time for DS1820 library to initialise
  wait(2);
}

DS18B20_TemperatureSensor::~DS18B20_TemperatureSensor()
{}

float DS18B20_TemperatureSensor::read(){
  probe.convertTemperature(true, DS1820::all_devices);
  int retry = SANITY_RETRY;
  float reading = probe.temperature();

  while(!isSane(reading) && retry > 0){
    wait_ms(100);
    reading = probe.temperature();
    retry--;
  }

  if(!isSane(reading)){ return SANITY_FAIL_VALUE; }

  lastReading = reading;
  
  return reading;
}

bool DS18B20_TemperatureSensor::isSane(float reading){
  return (reading < SANITY_MAX_VALUE) && (reading > SANITY_MIN_VALUE);
}
