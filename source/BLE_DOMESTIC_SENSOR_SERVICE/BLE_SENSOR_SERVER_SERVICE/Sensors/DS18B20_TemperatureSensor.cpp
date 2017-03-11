#include "DS18B20_TemperatureSensor.h"


DS18B20_TemperatureSensor::DS18B20_TemperatureSensor(PinName pin) :
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
  return probe.temperature();
    
}
