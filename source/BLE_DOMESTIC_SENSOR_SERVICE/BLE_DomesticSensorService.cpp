#include "BLE_DomesticSensorService.h"

DomesticSensorService::DomesticSensorService(BLE &ble, Serial *usbDebug, EventQueue *eventQueue) :
  SensorServerService(ble, usbDebug, eventQueue),
  debugger(usbDebug)
{
  //set up sensors - TODO make this an automatic process based on attached sensors
  //would be cool if could detect automatically
  //TEMPERATURE SENSOR
  /*
  PinName *pins = new PinName[1];
  pins[0] = p11;
  int numPins = 1;
  Sensor *temp = new DS18B20_TemperatureSensor(pins[0]);
  addSensor(temp, (uint16_t)5, (float) 0.0, TEMPERATURE, pins, numPins, 1024);
  */

  //RANGE FINDER
  PinName *pins = new PinName[2];
  pins[0] = p11; //ECHO
  pins[1] = p12; //TRIGGER
  Sensor * rangeFinder = new RangeFinder(pins[1], pins[0]);
  addSensor(rangeFinder, (uint16_t)5, (float)0.0, RANGE_FINDER, pins, 2, 1024);
    
  
}

DomesticSensorService::~DomesticSensorService(){}

void DomesticSensorService::configurationWriteCallback(uint16_t interval, uint32_t threshold){


}

void DomesticSensorService::stagingCommandWriteCallback(const uint8_t *command){


}

//Return bool from here? Allow that to yay/nay the read for any reason
void DomesticSensorService::stageBeforeReadCallback(){

  
  
}

