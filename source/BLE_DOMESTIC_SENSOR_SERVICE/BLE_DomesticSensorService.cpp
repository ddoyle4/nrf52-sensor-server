#include "BLE_DomesticSensorService.h"

DomesticSensorService::DomesticSensorService(BLE &ble, Serial *usbDebug, EventQueue *eventQueue) :
  SensorServerService(ble, usbDebug, eventQueue),
  debugger(usbDebug)
{
  //set up sensors - TODO make this an automatic process based on attached sensors
  //would be cool if could detect automatically
  //TEMPERATURE SENSOR
  PinName *pins = new PinName[1];
  pins[0] = p11;
  int numPins = 1;
  Sensor *newSensor = new DS18B20_TemperatureSensor(pins[0]);
  addSensor(newSensor, (uint16_t)5, (float) 0.25, TEMPERATURE, pins, numPins, 1024);
}

DomesticSensorService::~DomesticSensorService(){}

void DomesticSensorService::configurationWriteCallback(uint16_t interval, uint32_t threshold){


}

void DomesticSensorService::stagingCommandWriteCallback(const uint8_t *command){


}

//Return bool from here? Allow that to yay/nay the read for any reason
void DomesticSensorService::stageBeforeReadCallback(){

  
  
}

