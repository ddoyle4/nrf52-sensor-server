/**
 * @file   TemperatureStore.cpp
 * @author david <david@david-SATELLITE-L855>
 * @date   Fri Feb 10 23:58:52 2017
 * 
 * @brief  This class implements a simple circular buffer
 * for TemperatureRecord values. It handles provisioning 
 * sensible ID numbers and time delta values to each new
 * TemperatureRecord.
 * 
 * 
 */

#include "TemperatureStore.h"
#include <stdlib.h>
#include <iostream>

/** 
 * Creates the store and determines the number of records
 * it can accommodate based on the amount of memory allocated
 * to it.
 * 
 * @param memorySize size in KB allocated for this store
 */
TemperatureStore::TemperatureStore(int _memorySize) :
  top(0),
  bottom(0),
  memorySize(_memorySize)
{

  /* Size of TemperatureReadng will determine how many readings can be stored*/
  storeSize = (memorySize - HEADER_SIZE) / (sizeof(TemperatureRecord));
  
  //IMPORTANT NOTE: look into using mbed_ualloc() instead - https://docs.mbed.com/docs/getting-started-mbed-os/en/latest/Full_Guide/memory/#memory-allocation-in-mbed-os
  store = (TemperatureRecord *) malloc(sizeof(TemperatureRecord) * storeSize);

  stage = (uint8_t *) malloc(sizeof(uint8_t) * memorySize);

  formatStage();
}

void TemperatureStore::formatStage(){
  for(int i = 0; i < memorySize; i++){
    stage[i] = 0x00;
  }
}

TemperatureStore::~TemperatureStore(){
  free(store);
}

int TemperatureStore::getStoreSize(){
  return storeSize;
}

void TemperatureStore::addReading(float temp){

  
  if(getCurrentSize() > 0){ // Ignore time delta for first reading

  }
  TemperatureRecord reading(temp, getNextID(), 0);

  store[(top%storeSize)] = reading;

  top++;
  
  if ((top-bottom) > storeSize) { bottom++; }
  
}

/** 
 * Inspects the current store to return the next suitable
 * ID value, which will be (last_id + 1) % 2^8
 * 
 * 
 * @return uint8_t next suitable ID value
 */
uint8_t TemperatureStore::getNextID(){

  if(getCurrentSize() == 0) { return 0; }

  return (store[(top-1)%storeSize].getID() + 1) % 255;

}

int TemperatureStore::getCurrentSize(){
  return top-bottom;
}


/** 
 * Flushes all of the current temperature
 * readings to stage where they are
 * made available to BLE characteristic.
 *
 * Old data on stage is untouched - use
 * uint16_t at start of this space to 
 * determine what data is relevant.
 * 
 */
void TemperatureStore::flush() {

  
  int indexOffset = 0;

  //TODO this is nasty - consider limiting all class notions
  //of size to uint16_t
  uint16_t numRecords = (uint16_t) getCurrentSize();

  std::memcpy(&stage[indexOffset], &numRecords, 2);

  indexOffset += 2;
  
  for (int i = top-1;  i >= bottom; i--) {
    std::memcpy(&stage[indexOffset],
		getRecord(i%storeSize).getData(),
		TemperatureRecord::SIZEOF_RECORD);
    
    indexOffset += TemperatureRecord::SIZEOF_RECORD;
  }
}

const uint8_t * TemperatureStore::package() const {
  return stage;
}

/** 
 * Returns a specified number of records, starting
 * with most recent.
 * 
 * @param numRecords number of most recent records
 * to return
 * 
 * @return uint8_t * a number of TemperatureRecords
 */
TemperatureRecord TemperatureStore::getRecord(int index){
  return store[index];
}

void TemperatureStore::printStore(){
  std::cout << "\tID\tDELTA\tTEMP\n";
  for (int i = top-1; i >= bottom; i--) {
    store[(i%storeSize)].print();
    std::cout << "\n\r";
  }
}

