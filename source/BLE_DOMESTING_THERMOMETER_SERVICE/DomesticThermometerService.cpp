#include "DomesticThermometerService.h"


DomesticThermometerService::DomesticThermometerService(BLE &_ble) :
  ble(_ble),
  tempStore(MAX_CHARACTERISTIC_SIZE),
  readings(readingsUUID, (uint8_t *) tempStore.package())
{
  GattCharacteristic *dtChars[] = {&readings};
  GattService dtService(serviceUUID, dtChars, sizeof(dtChars) / sizeof(GattCharacteristic *));
  ble.addService(dtService);
  
}

void DomesticThermometerService::addTemperatureReading(float reading){
  tempStore.addReading(reading);
}

void DomesticThermometerService::flush(){
  tempStore.flush();
  const uint8_t * data = tempStore.package();
  ble.gattServer().write(readings.getValueHandle(), data, MAX_CHARACTERISTIC_SIZE);
}

