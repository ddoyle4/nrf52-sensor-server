#ifndef __BLE_SENSOR_SERVER_SERVICE__
#define __BLE_SENSOR_SERVER_SERVICE__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include <cstring>
#include "mbed.h"
#include "GattCharacteristic.h"
#include "GattCallbackParamTypes.h"
#include "SensorController.h"
#include "Sensors/SensorStore.h"
#include "Sensors/DS18B20_TemperatureSensor.h"
#include "Sensors/RangeFinder.h"
#include "Sensors/Sensor.h"
#include "Sensors/DebugSensor.h"


/* Tracks the last valid command to set data to stage */
struct active_read_command {
  command_type type;
  unsigned int startDelta, endDelta;
  uint8_t sensorID;
};

class SensorServerService {
 public:
  static const uint16_t SSS_UUID = 0xA000;
  static const unsigned int METADATA_SIZE = 36;
  static const uint16_t METADATA_UUID = 0xA001;
  static const unsigned int LIVEREAD_SIZE = 8*4;
  static const uint16_t LIVEREAD_UUID = 0xA002;
  static const unsigned int CONFIGURATION_SIZE = 48;
  static const uint16_t CONFIGURATION_UUID = 0xA003;
  static const unsigned int STAGINGCOMMAND_SIZE = 12;
  static const uint16_t STAGINGCOMMAND_UUID = 0xA004;
  static const unsigned int STAGE_SIZE = 500;
  static const uint16_t STAGE_UUID = 0xA005;

  SensorServerService(BLE &ble, Serial * debugger, EventQueue *eventQueue);
  ~SensorServerService();
  
  /* Metadata */
  void metadataFullCopy(uint8_t * newData);
  void metadataUpdateMaxBufferSize(uint16_t maxSize, uint8_t sensorID);
  void metadataUpdateCurrentBufferSize(uint16_t newSize, uint8_t sensorID);
  void metadataUpdateSensorType(uint8_t sensorID, uint8_t sensorType);

  /* Live Read */
  void liveReadUpdate(float reading, int sensorID);

  /* Configuration */
  virtual void configurationWriteCallback(uint16_t interval, uint32_t threshold) =0;
  void configUpdate(uint8_t sensorID, uint16_t interval, float threshold);
  
  /* Staging Command */
  virtual void stagingCommandWriteCallback(const uint8_t * command) =0;

  /* Stage */
  uint8_t * getStageData() { return stage_data; }
  void flushStageData(unsigned int oldestLimit, unsigned int youngestLimit, uint8_t sensor, command_type ctype);
  virtual void stageBeforeReadCallback() =0;

  /* Callback Functions */
  void writeCallback(const GattWriteCallbackParams * params);
  void stageReadCallback(GattReadAuthCallbackParams * params);
  void liveReadCallback(GattReadAuthCallbackParams * params);
  void metadataCallback(GattReadAuthCallbackParams * params);

  int addSensor(Sensor *sensor, uint16_t interval, float threshold, sensorType _type, PinName *pins, int numPins, int memSize);
  
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
  ReadOnlyArrayGattCharacteristic<uint8_t, CONFIGURATION_SIZE> configuration_charac;
  ReadWriteArrayGattCharacteristic<uint8_t, STAGINGCOMMAND_SIZE> stagingCommand_charac;
  ReadOnlyArrayGattCharacteristic<uint8_t, STAGE_SIZE> stage_charac;

  /* NOTE: data will always be of length STAGE_COMMAND_SIZE. Any unused bytes at the end 
     are set to 0x00*/
  void stageCommandHandler(const uint8_t *data);
  void configUpdateHandler(uint8_t sensorID, uint16_t interval, float threshold);

  active_read_command activeReadCommand;

  /* For sequential reads*/
  bool slideReadWindow();
};



#endif

