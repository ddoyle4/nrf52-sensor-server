#include "DS18B20_TemperatureSensor.h"


DS18B20_TemperatureSensor::DS18B20_TemperatureSensor(PinName pin) :
  assignedPin(pin),
  probe(DS1820(pin))
{

}

DS18B20_TemperatureSensor::~DS18B20_TemperatureSensor()
{}

float DS18B20_TemperatureSensor::read(){
  probe.convertTemperature(true, DS1820::all_devices);
  return probe.temperature();
    
}
