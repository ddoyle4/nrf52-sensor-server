
#ifndef __BLE_DOMESTIC_SENSOR_SERVICE__
#define __BLE_DOMESTIC_SENSOR_SERVICE__

#include "BLE_SENSOR_SERVER_SERVICE/BLE_SensorServerService.h"
#include "ble/BLE.h"
#include "mbed.h"

class DomesticSensorService : SensorServerService {
 public:
  DomesticSensorService(BLE &ble, EventQueue *eventQueue);
  ~DomesticSensorService();

 private:
  void configurationWriteCallback(uint16_t interval, uint32_t threshold);
  void stagingCommandWriteCallback(const uint8_t * command);
  void stageBeforeReadCallback();
};



#endif
