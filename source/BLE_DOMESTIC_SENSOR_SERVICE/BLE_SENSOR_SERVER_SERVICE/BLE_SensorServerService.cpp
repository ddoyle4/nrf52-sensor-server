#include "BLE_SensorServerService.h"

// see spec for metadata format
uint8_t SensorServerService::metadata_data[METADATA_SIZE] = { 
  // start with max buffer of 0
  0x00, 0x00,

  // start with sensor type of NON_A_SENSOR
  0x77, 0x77, 0x77, 0x77,

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
  sensorController(_debugger, eventQueue),
  metadata_charac(METADATA_UUID, metadata_data),
  liveRead_charac(LIVEREAD_UUID, liveRead_data),
  configuration_charac(CONFIGURATION_UUID, configuration_data),
  stagingCommand_charac(STAGINGCOMMAND_UUID, stagingCommand_data),
  stage_charac(STAGE_UUID, stage_data)
{

  //set up BLE service
  GattCharacteristic *SSSChars[] = {&metadata_charac,
				    &liveRead_charac,
				    &configuration_charac,
				    &stagingCommand_charac,
				    &stage_charac};

  // Called before stage read is allowed to proceed
  liveRead_charac.setReadAuthorizationCallback(this, &SensorServerService::liveReadCallback);
  
  stage_charac.setReadAuthorizationCallback(this, &SensorServerService::stageReadCallback);
  GattService SSSService(SSS_UUID, SSSChars, sizeof(SSSChars) / sizeof(GattCharacteristic *));
  ble.addService(SSSService);

  // All write callbacks
  ble.gattServer().onDataWritten(this, &SensorServerService::writeCallback);

  // Initial Metadata Values
  metadataUpdateMaxBufferSize(sensorController.getMaxBufferSize());
}

SensorServerService::~SensorServerService(){}

void SensorServerService::metadataFullCopy(uint8_t * newData){
  std::memcpy(&metadata_data, newData, sizeof(METADATA_SIZE));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateMaxBufferSize(uint16_t maxBuffer){
  std::memcpy(&metadata_data[0], &maxBuffer, sizeof(uint16_t));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateSensorBufferSize(uint16_t newSize, uint8_t sensorID){
  unsigned int sizeOffset = 6, sizeLength = 2;
  sizeOffset = sizeOffset + (sizeLength * sensorID);
  std::memcpy(&metadata_data, &newSize, sizeof(uint16_t));
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
  unsigned int typeOffset = 2;
  typeOffset = typeOffset + (sensorID / (uint8_t)2);

  int test = (int)sensorType;
  debugger->printf("type is %d\n\r", test);
  uint8_t metadataByte = metadata_data[typeOffset];
  
  if( sensorID % (uint8_t)2){
    metadataByte = ((metadataByte) & 0xF0) | (sensorType & 0x0F);
  } else {
    metadataByte = ((metadataByte) & 0x0F) | (sensorType << 4);
  }

  test = (int)metadataByte;
  debugger->printf("byte is %d\n\r", test);
  metadata_data[typeOffset] = metadataByte;
  
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::liveReadUpdate(float reading, int sensorID){
  int indexOffset = 4 * sensorID;
  std::memcpy(&liveRead_data[indexOffset], &reading, 4);
  const uint8_t * liveRead = liveRead_data;
  ble.gattServer().write(liveRead_charac.getValueHandle(), liveRead, 4);
}

void SensorServerService::liveReadCallback(GattReadAuthCallbackParams *params){
  for(int i=0; i < sensorController.getNumSensors(); i++){
    float reading = sensorController.getSensor(i)->read();
    liveReadUpdate(reading, i);
  }
  
}

void SensorServerService::stageReadCallback(GattReadAuthCallbackParams *params){
  stageBeforeReadCallback();
}


void SensorServerService::writeCallback(const GattWriteCallbackParams *params){
  if (params->handle == configuration_charac.getValueHandle()){
    uint16_t interval = 1;
    uint32_t threshold = 1;

    configurationWriteCallback(interval, threshold);
  } else if(params->handle == stagingCommand_charac.getValueHandle()){

    stageCommandHandler(params->data);
    //call virt func to allow derived class to perform any other required processing
    stagingCommandWriteCallback(params->data);
  }
}

void SensorServerService::flushStageData(unsigned int oldestLimit, unsigned int youngLimit, uint8_t sensor){
  unsigned int sizeStage = sensorController.flushSensorStore(oldestLimit, youngLimit, sensor);
  ble.gattServer().write(stage_charac.getValueHandle(), sensorController.getPackage(sensor), sizeStage);
}

void SensorServerService::stageCommandHandler(const uint8_t *data){

  switch(data[0]){

  case 0x00: //Stage Command
    unsigned int oldLimit, youngLimit;

    std::memcpy(&oldLimit, &data[1], sizeof(unsigned int));
    std::memcpy(&youngLimit, &data[5], sizeof(unsigned int));
    debugger->printf("STAGE COMMAND: %d %d %d\n\r", oldLimit, youngLimit, data[9]);
    flushStageData(oldLimit, youngLimit, data[9]);
    break;


  default:
    break;
  }
}

int SensorServerService::addSensor(Sensor *sensor, uint16_t interval, sensorType type, PinName *pins, int numPins){

  int newSensorID = sensorController.addSensor(sensor, interval, type, pins, numPins);

  //TODO tidy this up
  newSensorID = (newSensorID > 15) ? 15 : newSensorID;
  if(newSensorID >= 0){
    //debugger->printf("new sensor id = %d and type = %d", newSensorID, type);
    metadataUpdateSensorType((uint8_t)newSensorID, (uint8_t)type);
    return newSensorID;
  }

  return -1;
}

