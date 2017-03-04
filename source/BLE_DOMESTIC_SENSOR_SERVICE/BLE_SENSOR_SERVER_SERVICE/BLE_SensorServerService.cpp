#include "BLE_SensorServerService.h"

uint8_t SensorServerService::metadata_data[METADATA_SIZE] = {0};
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
  liveRead_charac.setReadAuthorizationCallback(this, &SensorServerService::liveReadCallback);
  stage_charac.setReadAuthorizationCallback(this, &SensorServerService::stageReadCallback);
  GattService SSSService(SSS_UUID, SSSChars, sizeof(SSSChars) / sizeof(GattCharacteristic *));
  ble.addService(SSSService);
  ble.gattServer().onDataWritten(this, &SensorServerService::writeCallback);

}

SensorServerService::~SensorServerService(){}

void SensorServerService::metadataFullCopy(uint8_t * newData){
  std::memcpy(metadata_data, newData, sizeof(METADATA_SIZE));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateCurrentBufferSize(uint16_t newSize){
  metadata_data[2] = newSize & 0xFF;
  metadata_data[3] = newSize >> 8;
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateLiveliness(uint8_t newLiveliness){
  metadata_data[4] = newLiveliness;
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
  unsigned int sizeStage = sensorController.flushSenstore(oldestLimit, youngLimit, sensor);
  ble.gattServer().write(stage_charac.getValueHandle, sensorController.getPackage(sensor), sizeStage);
}

void SensorServerService::stageCommandHandler(uint8_t *data){

  switch(data[0]){

  case 0x00: //Stage Command
    unsigned int oldLimit, youngLimit;
    std::memcpy(&oldLimit, &data[1], sizeof(unsigned int));
    std::memcpy(&youngLimit, &data[5], sizeof(unsigned int));
    flushStageData(oldLimit, youngLimit, data[9]);
    break;


  default:
  }
}


