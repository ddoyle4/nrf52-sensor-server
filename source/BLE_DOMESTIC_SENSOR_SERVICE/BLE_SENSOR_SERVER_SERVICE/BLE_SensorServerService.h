#ifndef __BLE_SENSOR_SERVER_SERVICE__
#define __BLE_SENSOR_SERVER_SERVICE__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstring>
#include "mbed.h"
#include "GattCharacteristic.h"
#include "GattCallbackParamTypes.h"
#include "SensorController.h"
#include "Sensors/DS18B20_TemperatureSensor.h"
#include "Sensors/Sensor.h"

class SensorServerService {
 public:
  static const uint16_t SSS_UUID = 0xA000;
  static const unsigned int METADATA_SIZE = 6;
  static const uint16_t METADATA_UUID = 0xA001;
  static const unsigned int LIVEREAD_SIZE = 4;
  static const uint16_t LIVEREAD_UUID = 0xA002;
  static const unsigned int CONFIGURATION_SIZE = 6;
  static const uint16_t CONFIGURATION_UUID = 0xA003;
  static const unsigned int STAGINGCOMMAND_SIZE = 12;
  static const uint16_t STAGINGCOMMAND_UUID = 0xA004;
  static const unsigned int STAGE_SIZE = 500;
  static const uint16_t STAGE_UUID = 0xA005;

  SensorServerService(BLE &ble, Serial * debugger, EventQueue *eventQueue);
  ~SensorServerService();


  
  /* Metadata */
  void metadataFullCopy(uint8_t * newData);
  void metadataUpdateCurrentBufferSize(uint16_t newSize);
  void metadataUpdateLiveliness(uint8_t newLiveliness);

  /* Live Read */
  void liveReadUpdate(uint8_t * newRead);

  /* Configuration */
  virtual void configurationWriteCallback(uint16_t interval, uint32_t threshold) =0;

  /* Staging Command */
  virtual void stagingCommandWriteCallback(const uint8_t * command) =0;

  /* Stage */
  uint8_t * stageData() { return stage_data; }
  virtual void stageBeforeReadCallback() =0;

  /* Callback Functions */
  void writeCallback(const GattWriteCallbackParams * params);
  void readCallback(GattReadAuthCallbackParams * params);
  
 protected:
  BLE &ble;
  Serial *debugger;
  SensorController sensorController;
  
 private:
  static uint8_t metadata_data[METADATA_SIZE];
  static uint8_t liveRead_data[LIVEREAD_SIZE];
  static uint8_t configuration_data[CONFIGURATION_SIZE];
  static uint8_t stagingCommand_data[STAGINGCOMMAND_SIZE];
  static uint8_t stage_data[STAGE_SIZE];

  ReadOnlyArrayGattCharacteristic<uint8_t, METADATA_SIZE> metadata_charac;
  ReadOnlyArrayGattCharacteristic<uint8_t, LIVEREAD_SIZE> liveRead_charac;
  ReadWriteArrayGattCharacteristic<uint8_t, CONFIGURATION_SIZE> configuration_charac;
  ReadWriteArrayGattCharacteristic<uint8_t, STAGINGCOMMAND_SIZE> stagingCommand_charac;
  ReadOnlyArrayGattCharacteristic<uint8_t, STAGE_SIZE> stage_charac;
 
};



#endif

