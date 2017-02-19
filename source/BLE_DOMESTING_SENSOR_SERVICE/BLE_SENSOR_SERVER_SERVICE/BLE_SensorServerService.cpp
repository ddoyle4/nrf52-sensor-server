#include "BLE_SensorServerService.h"

SensorServerService::SensorServerService(BLE &_ble) :
  ble(_ble),
  metadata_charac(METADATA_UUID, metadata_data),
  liveRead_charac(LIVEREAD_UUID, liveRead_data),
  configuration_charac(CONFIGURATION_UUID, configuration_data),
  stagingCommand_charac(STAGINGCOMMAND_UUID, stagineCommand_data),
  stage_charac(STAGE_UUID, stage_data)
{
  GattCharacteristic *SSSChars[] = {&metadata_charac,
				    &liveRead_charac,
				    $configuration_charac,
				    &stagingCommand_charac,
				    &stage_charac};

  GattService SSSService(SSS_UUID, SSSChars, sizeof(SSSChars) / sizeof(GattCharacteristic *));
  ble.addService(SSSService);
  ble.gattServer().onDataWritten(writeCallback);
  ble.gattServer().setReadAuthorizationCallback(readCallback);
}

SensorServerService::~SensorServerService(){}

void SensorServerService::metadataFullCopy(uint8_t * newData){
  std::memcpy(metadata_data, newData, sizeof(METADATA_SIZE));
  const uint8_t * metadata = metadata_data;
  ble.gattServer().write(metadata_charac.getValueHandle(), metadata, METADATA_SIZE);
}

void SensorServerService::metadataUpdateCurrentBufferSize(uint16_t newSize){
  std::memcpy(metadata_data[2], &newSize, sizeof(newSize));
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

void SensorServerService::readCallback(const GattReadAuthCallbackParams *params){
  if (params->handle == stage_charac.getValueHandle()){
    stageBeforeReadCallback();
  }
}

void SensorServerService::writeCallback(const GattWriteCallbackParams *params){
  if (parmas->handle == configuration_charac.getValueHandle()){
    uint16_t interval;
    uint32_t threshold;

    std::memcpy(&interval, params->data[0], sizeof(interval));
    std::memcpy(&threshold, params->data[2], sizeof(threshold));

    configurationWriteCallback(interval, threshold);
  } else if(params->handle == stagingCommand_charac.getValueHandle()){
    stagingCommandWriteCallback(params->data);
  }
}
