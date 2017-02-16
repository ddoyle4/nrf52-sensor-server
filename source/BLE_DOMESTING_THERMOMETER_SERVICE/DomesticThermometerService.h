
#ifndef __BLE_DOMESTIC_THERMOMETER_H__
#define __BLE_DOMESTIC_THERMOMETER_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "TemperatureStore.h"

class DomesticThermometerService {

 public:
  static const uint16_t serviceUUID = 0xA000;
  static const uint16_t readingsUUID = 0xA00A;
  static const unsigned MAX_CHARACTERISTIC_SIZE = 300;
    
  DomesticThermometerService(BLE &ble);
  void addTemperatureReading(float reading);
  void flush();
  
 private:
  BLE &ble;
  TemperatureStore tempStore;
  ReadOnlyArrayGattCharacteristic<uint8_t, MAX_CHARACTERISTIC_SIZE> readings;


   
};

#endif
