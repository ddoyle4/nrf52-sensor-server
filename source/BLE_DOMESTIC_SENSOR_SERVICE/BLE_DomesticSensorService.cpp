#include "BLE_DomesticSensorService.h"

DomesticSensorService::DomesticSensorService(BLE &ble, Serial *usbDebug) :
  SensorServerService(ble, usbDebug),
  debugger(usbDebug)
{
  debugger->printf("in dss\n\r");
}

DomesticSensorService::~DomesticSensorService(){}

void DomesticSensorService::configurationWriteCallback(uint16_t interval, uint32_t threshold){
  debugger->printf("CONFIG WRITE CALLBACK\n\r");
  metadataUpdateCurrentBufferSize(interval);
  //  led2 = !led2;
}

void DomesticSensorService::stagingCommandWriteCallback(const uint8_t *command){
  //  led3 = !led3;
}
/*
void DomesticSensorService::stageBeforeReadCallback(){
  //  led4 = !led4;
}
*/
