#include "BLE_DomesticSensorService.h"

DomesticSensorService::DomesticSensorService(BLE &ble, Serial *usbDebug, EventQueue *eventQueue) :
  SensorServerService(ble, usbDebug, eventQueue),
  debugger(usbDebug)
{
  //set up sensors - TODO make this an automatic process based on attached sensors
  //would be cool if could detect automatically

  //TEMPERATURE SENSOR
  PinName *tpins = new PinName[1];
  tpins[0] = p11;
  int numPins = 1;
  Sensor *temp = new DS18B20_TemperatureSensor(tpins[0]);
  addSensor(temp, (uint16_t)5, (float) 0.0, TEMPERATURE, tpins, numPins, 80);

  //RANGE FINDER
  PinName *rfpins = new PinName[2];
  rfpins[0] = p12; 
  rfpins[1] = p13;
  /*                                      trigger   echo  */
  Sensor * rangeFinder = new RangeFinder(rfpins[1], rfpins[0]);
  addSensor(rangeFinder, (uint16_t)5, (float)0.0, RANGE_FINDER, rfpins, 2, 80);
}

DomesticSensorService::~DomesticSensorService(){}

void DomesticSensorService::configurationWriteCallback(uint16_t interval, uint32_t threshold){}

void DomesticSensorService::stagingCommandWriteCallback(const uint8_t *command){}

//Return bool from here? Allow that to yay/nay the read for any reason
void DomesticSensorService::stageBeforeReadCallback(){}

