#include "DomesticThermometerService.h"
#include "mbed.h"

DigitalOut led(LED2, 1);


uint8_t DomesticThermometerService::commandValue[12] = {0};

DomesticThermometerService::DomesticThermometerService(BLE &_ble) :
  ble(_ble),
  tempStore(MAX_CHARACTERISTIC_SIZE),
  readings(readingsUUID, (uint8_t *) tempStore.package()),
  command(commandUUID, commandValue)
{
  GattCharacteristic *dtChars[] = {&readings, &command};
  GattService dtService(serviceUUID, dtChars, sizeof(dtChars) / sizeof(GattCharacteristic *));
  ble.addService(dtService);
  ble.gattServer().onDataWritten(writeCallBack);
  
}

void DomesticThermometerService::addTemperatureReading(float reading){
  tempStore.addReading(reading);
}

void DomesticThermometerService::flush(){
  tempStore.flush();
  const uint8_t * data = tempStore.package();
  ble.gattServer().write(readings.getValueHandle(), data, MAX_CHARACTERISTIC_SIZE);
}

void DomesticThermometerService::writeCallBack(const GattWriteCallbackParams *params){
  led = params->data[0];
}
