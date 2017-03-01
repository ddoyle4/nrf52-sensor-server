#include "BLE_SensorServerService.h"

uint8_t SensorServerService::metadata_data[METADATA_SIZE] = {0};
uint8_t SensorServerService::liveRead_data[LIVEREAD_SIZE] = {0};
uint8_t SensorServerService::configuration_data[CONFIGURATION_SIZE] = {0};
uint8_t SensorServerService::stagingCommand_data[STAGINGCOMMAND_SIZE] = {0};
uint8_t SensorServerService::stage_data[STAGE_SIZE] = {0};

SensorServerService::SensorServerService(BLE &_ble, Serial *_debugger) :
  ble(_ble),
  debugger(_debugger),
  metadata_charac(METADATA_UUID, metadata_data),
  liveRead_charac(LIVEREAD_UUID, liveRead_data),
  configuration_charac(CONFIGURATION_UUID, configuration_data),
  stagingCommand_charac(STAGINGCOMMAND_UUID, stagingCommand_data),
  stage_charac(STAGE_UUID, stage_data),
  sensorController(_debugger)
{
  GattCharacteristic *SSSChars[] = {&metadata_charac,
				    &liveRead_charac,
				    &configuration_charac,
				    &stagingCommand_charac,
				    &stage_charac};
  stage_charac.setReadAuthorizationCallback(this, &SensorServerService::readCallback);
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

void SensorServerService::liveReadUpdate(uint8_t *newRead){
  std::memcpy(liveRead_data, newRead, LIVEREAD_SIZE);
  const uint8_t * liveRead = liveRead_data;
  ble.gattServer().write(liveRead_charac.getValueHandle(), liveRead, LIVEREAD_SIZE);
}

void SensorServerService::readCallback(GattReadAuthCallbackParams *params){
  stageBeforeReadCallback();
  if (params->handle == stage_charac.getValueHandle()){
    stageBeforeReadCallback();
  }
}


void SensorServerService::writeCallback(const GattWriteCallbackParams *params){
  if (params->handle == configuration_charac.getValueHandle()){
    uint16_t interval = 1;
    uint32_t threshold = 1;

    configurationWriteCallback(interval, threshold);
  } else if(params->handle == stagingCommand_charac.getValueHandle()){
    stagingCommandWriteCallback(params->data);
  }
}


