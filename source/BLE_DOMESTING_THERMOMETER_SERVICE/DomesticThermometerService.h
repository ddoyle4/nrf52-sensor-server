
#ifndef __BLE_DOMESTIC_THERMOMETER_H__
#define __BLE_DOMESTIC_THERMOMETER_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "TemperatureStore.h"

class DomesticThermometerService {

 public:
  static const uint16_t serviceUUID = 0xA000;
  static const uint16_t readingsUUID = 0xA00A;
  static const uint16_t commandUUID = 0xA009;
  static const unsigned MAX_CHARACTERISTIC_SIZE = 300;
    
  DomesticThermometerService(BLE &ble);
  void addTemperatureReading(float reading);
  void flush();
  static void writeCallBack(const GattWriteCallbackParams *params);

 private:
  BLE &ble;
  TemperatureStore tempStore;
  static uint8_t commandValue[12];
  ReadOnlyArrayGattCharacteristic<uint8_t, MAX_CHARACTERISTIC_SIZE> readings;
  WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(commandValue)> command;
};

#endif
