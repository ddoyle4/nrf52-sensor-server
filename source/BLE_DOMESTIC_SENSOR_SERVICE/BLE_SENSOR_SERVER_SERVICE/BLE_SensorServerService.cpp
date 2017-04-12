#include "BLE_SensorServerService.h"

// see spec for metadata format
uint8_t SensorServerService::metadata_data[METADATA_SIZE] = { 
  // start with sensor type of NON_A_SENSOR
  0x77, 0x77, 0x77, 0x77,

  //start with max buffer size of 0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

  //start with current buffer size of 0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t SensorServerService::liveRead_data[LIVEREAD_SIZE] = {0};
uint8_t SensorServerService::configuration_data[CONFIGURATION_SIZE] = {0};
uint8_t SensorServerService::stagingCommand_data[STAGINGCOMMAND_SIZE] = {0};
uint8_t SensorServerService::stage_data[STAGE_SIZE] = {0};

SensorServerService::SensorServerService(BLE &_ble, Serial *_debugger, EventQueue *eventQueue) :
  ble(_ble),
  debugger(_debugger),
  sensorController(_debugger, eventQueue, STAGE_SIZE),
  metadata_charac(METADATA_UUID, metadata_data),
  liveRead_charac(LIVEREAD_UUID, liveRead_data),
  configuration_charac(CONFIGURATION_UUID, configuration_data),
  stagingCommand_charac(STAGINGCOMMAND_UUID, stagingCommand_data),
  stage_charac(STAGE_UUID, stage_data),
  activeCommand(READ_STATIC)
{

  //set up BLE service
  GattCharacteristic *SSSChars[] = {&metadata_charac,
				    &liveRead_charac,
				    &configuration_charac,
				    &stagingCommand_charac,
				    &stage_charac};
  // Read Authorisation callbacks
  metadata_charac.setReadAuthorizationCallback(this, &SensorServerService::metadataCallback);
  liveRead_charac.setReadAuthorizationCallback(this, &SensorServerService::liveReadCallback);  
  stage_charac.setReadAuthorizationCallback(this, &SensorServerService::stageReadCallback);
  
  GattService SSSService(SSS_UUID, SSSChars, sizeof(SSSChars) / sizeof(GattCharacteristic *));
  ble.addService(SSSService);

  // All write callbacks
  ble.gattServer().onDataWritten(this, &SensorServerService::writeCallback);

}

SensorServerService::~SensorServerService(){}

void SensorServerService::metadataFullCopy(uint8_t * newData){
  std::memcpy(&metadata_data, newData, sizeof(METADATA_SIZE));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateMaxBufferSize(uint16_t maxBuffer, uint8_t sensorID){
  unsigned int maxSizeOffset = 4, sizeLength = 2;
  maxSizeOffset = maxSizeOffset + (sensorID * sizeLength);
  std::memcpy(&metadata_data[maxSizeOffset], &maxBuffer, sizeof(uint16_t));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateCurrentBufferSize(uint16_t newSize, uint8_t sensorID){
  unsigned int sizeOffset = 20, sizeLength = 2;
  sizeOffset = sizeOffset + (sizeLength * sensorID);
  std::memcpy(&metadata_data[sizeOffset], &newSize, sizeof(uint16_t));

  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

/** 
 * Updates the metadata appropriately to set the sensor type for a given sensor.
 * 
 * @param sensorID The sensor being updated
 * @param sensorType The type to set the sensor to - 4 least significant bits
 */
void SensorServerService::metadataUpdateSensorType(uint8_t sensorID, uint8_t sensorType){
  unsigned int typeOffset = 0;
  typeOffset = typeOffset + (sensorID / (uint8_t)2);

  uint8_t metadataByte = metadata_data[typeOffset];
  
  if( sensorID % (uint8_t)2){
    metadataByte = ((metadataByte) & 0x0F) | (sensorType << 4);
  } else {
    metadataByte = ((metadataByte) & 0xF0) | (sensorType & 0x0F);
  }

  metadata_data[typeOffset] = metadataByte;
  
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataCallback(GattReadAuthCallbackParams *params){
  for(int i=0; i < sensorController.getNumSensors(); i++){
    int size = sensorController.getSensorStore(i)->getCurrentSize();
    uint16_t updateSize = (size > 65536) ? 0xFFFF : (uint16_t)size;
    metadataUpdateCurrentBufferSize(updateSize, (uint8_t)i);
  }
}

void SensorServerService::liveReadUpdate(float reading, int sensorID){
  int indexOffset = 4 * sensorID;
  std::memcpy(&liveRead_data[indexOffset], &reading, 4);
  const uint8_t * liveRead = liveRead_data;
  ble.gattServer().write(liveRead_charac.getValueHandle(), liveRead, LIVEREAD_SIZE);
}

void SensorServerService::liveReadCallback(GattReadAuthCallbackParams *params){
  for(int i=0; i < sensorController.getNumSensors(); i++){
    float reading = sensorController.getSensor(i)->read();
    liveReadUpdate(reading, i);
  }
}

void SensorServerService::configUpdate(uint8_t sensorID, uint16_t interval, float threshold){
  int indexOffset = 6 * sensorID;
  std::memcpy(&configuration_data[indexOffset], &interval, sizeof(uint16_t));
  std::memcpy(&configuration_data[indexOffset+2], &threshold, sizeof(float));
  const uint8_t * config = configuration_data;
  ble.gattServer().write(configuration_charac.getValueHandle(), config, CONFIGURATION_SIZE);
}

void SensorServerService::stageReadCallback(GattReadAuthCallbackParams *params){
  //TODO check bool return status and yay/nay the read for that

  //NOTE this is called multiple times for each stage read - looks like it's called
  //to authorise each chunk of data being sent. Must update the start time only once
  if(params->offset == 0){

    sensorController.updateStageStartTime();
    ble.gattServer().write(stage_charac.getValueHandle(), sensorController.getPackage(), STAGE_SIZE);
  }

  //  stageBeforeReadCallback();
}


void SensorServerService::writeCallback(const GattWriteCallbackParams *params){
  if(params->handle == stagingCommand_charac.getValueHandle()){ 
    stageCommandHandler(params->data);
    stagingCommandWriteCallback(params->data);
  }
}

void SensorServerService::flushStageData(unsigned int oldestLimit, unsigned int youngLimit, uint8_t sensor){
  sensorController.flushSensorStore(oldestLimit, youngLimit, sensor);
  ble.gattServer().write(stage_charac.getValueHandle(), sensorController.getPackage(), STAGE_SIZE);
}

/** 
 * The command is identified by the first byte. All commands beginning with
 * 0x0 are requests for data. All commands beginning with 0x1 are requests to
 * change the configuration.
 * 
 * @param data raw command byte array
 */
void SensorServerService::stageCommandHandler(const uint8_t *data){

  switch(data[0]){
  case READ_STATIC: //STATIC READ
    activeCommand = READ_STATIC;
    
    unsigned int oldLimit, youngLimit;
    
    std::memcpy(&oldLimit, &data[1], sizeof(unsigned int));
    std::memcpy(&youngLimit, &data[5], sizeof(unsigned int));
    flushStageData(oldLimit, youngLimit, data[9]);
    break;

  case CONFIG_WRITE: //Update config
    uint16_t newInterval;
    float newThreshold;
    
    std::memcpy(&newInterval, &data[2], sizeof(uint16_t));
    std::memcpy(&newThreshold, &data[4], sizeof(float));

    configUpdateHandler(data[1], newInterval, newThreshold);
    break;
  default: /* TODO: Display error code on stage here*/
    break;
  }
}

void SensorServerService::configUpdateHandler(uint8_t sensorID, uint16_t interval, float threshold){
  sensorController.getSensorStore(sensorID)->setThreshold(threshold);

  //update config characteristic
  configUpdate(sensorID, interval, threshold);
}

int SensorServerService::addSensor(Sensor *sensor, uint16_t interval, float threshold, sensorType type, PinName *pins, int numPins, int memSize){
  int newSensorID = sensorController.addSensor(sensor, interval, threshold, type, pins, numPins, memSize);
  //TODO tidy this up
  newSensorID = (newSensorID > 15) ? 15 : newSensorID;
  if(newSensorID >= 0){
    metadataUpdateSensorType((uint8_t)newSensorID, (uint8_t)type);
    metadataUpdateMaxBufferSize((uint16_t)sensorController.getSensorStore(newSensorID)->getStoreSize(), newSensorID);
    configUpdate((uint8_t)newSensorID, interval, threshold);
      
    return newSensorID;
  }

  return -1;
}

