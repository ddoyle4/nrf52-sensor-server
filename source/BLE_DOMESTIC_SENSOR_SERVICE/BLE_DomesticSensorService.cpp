#include "BLE_DomesticSensorService.h"

DomesticSensorService::DomesticSensorService(BLE &ble, Serial *usbDebug) :
  SensorServerService(ble, usbDebug),
  debugger(usbDebug)
{
}

DomesticSensorService::~DomesticSensorService(){}

void DomesticSensorService::configurationWriteCallback(uint16_t interval, uint32_t threshold){
  metadataUpdateCurrentBufferSize(0xFFFF);
  //  led2 = !led2;
}

void DomesticSensorService::stagingCommandWriteCallback(const uint8_t *command){
  metadataUpdateCurrentBufferSize(0x0000);
  //  led3 = !led3;
}

//Return bool from here? Allow that to yay/nay the read for any reason
void DomesticSensorService::stageBeforeReadCallback(){
  metadataUpdateCurrentBufferSize(0xFFFF);
  //  led4 = !led4;
}

