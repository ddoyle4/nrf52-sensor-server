#include "SensorBufferStore.h"
#include <stdlib.h>
#include <iostream>

SensorStore::SensorStore(int _memorySize, int _stageSize, uint16_t interval) :
  top(0),
  bottom(0),
  memorySize(_memorySize),
  measurementInterval(interval),
  stageSize(_stageSize)
{

  /* Size of SensorRecord will determine how many readings can be stored*/
  storeSize = memorySize / (sizeof(SensorRecord));
  
  //IMPORTANT NOTE: look into using mbed_ualloc() instead - https://docs.mbed.com/docs/getting-started-mbed-os/en/latest/Full_Guide/memory/#memory-allocation-in-mbed-os
  store = (SensorRecord *) malloc(sizeof(SensorRecord) * storeSize);

  //Area in memory where records will be written to so they can easily be written
  //to the gatt server.
  stage = (uint8_t *) malloc(sizeof(uint8_t) * stageSize);

  formatStage();
}

void SensorStore::formatStage(){
  for(int i = 0; i < stageSize; i++){
    stage[i] = 0x00;
  }
}

SensorStore::~SensorStore(){
  free(store);
  free(stage);
}

int SensorStore::getStoreSize(){
  return storeSize;
}

int SensorStore::getStageSize(){
  return stageSize;
}

//when adding threshold - must check if time is about
//to pass the measurementInterval max value - in which case
//should add the reading regardless of threshold so as to be able
//to mainting accurate relative readings
void SensorStore::addReading(float value){

  uint16_t timeDelta = 0;
  time_t currentTime = time(NULL);

  if(getCurrentSize() > 0){ // Ignore time delta for first reading
    double timeDiff = difftime(currentTime, lastReadingTime);
    unsigned long delta = ((timeDiff / measurementInterval) + 0.5);
    timeDelta = (delta > 65535) ? 0xFFFF : (uint16_t)delta;
  }

  lastReadingTime = currentTime;

  SensorRecord reading(value, timeDelta);

  store[(top%storeSize)] = reading;

  top++;
  
  if ((top-bottom) > storeSize) { bottom++; }
  
}

int SensorStore::getCurrentSize(){
  return top-bottom; //top points to next write location
}

/** 
 * Writes as many records as possible to the stage
 * depending on size of stage and size of record.
 * 
 * 
 * 
 * 
 * @return 
 *
unsigned int SensorStore::flush(int) {
  
  int indexOffset = 0;  

  for (int i = top-1;  i >= bottom; i--) {
    std::memcpy(&stage[indexOffset],
		getRecord(i%storeSize).getData(),
		TemperatureRecord::SIZE_RECORD);
    
    indexOffset += TemperatureRecord::SIZE_RECORD;
  }
  } */
/** 
 * Stores in stage all records within the time period specified.
 * Where there are more records than stage space, the oldest records
 * are prioritised.
 * The number of seconds that have past since the first record stored on the stage
 * will be saved in the first four bytes of the stage. After this, records
 * have the following format:
 *                      2 bytes          4 bytes
 *                   | timeDelta | measurementReading |
 * where
 *       timeDelta:          is the amount of time, measured in number of 
 *                           measurementIntervals, since the reading immediately 
 *                           previous.
 *       measurementReading: is the actual reading value.
 *
 * @param oldestTime exclude readings before this time
 * @param youngestTime exclude readings after this time
 * 
 * @return the number of records that were written to the stage
 */
unsigned int SensorStore::flush(unsigned int oldestTime, unsigned int youngestTime){
  unsigned int index = bottom,
    relationalDelta,
    realTImeDelta,
    prevRealTimeDelta;

  //realTimeDelta of record immediately before bottom (may not exist)
  prevRealTimeDelta = getOldestRecordTimeDelta() -
    (getRecord(bottom%storeSize).getTimeDelta() * measurementInterval)
    
  SensorRecord currentRecord;     

  stack<SenssorRecord> records;
  
  while(index < top){
    currentRecord = getRecord(index%storeSize);
    //amount of time (secs) between this record and previous record
    relationalDelta = currentRecord.getTimeDelta()*measurementInterval;
    //amount of time (secs) between now and when this record was stored
    realTimeDelta = relationalDelta + prevRealTimeDelta;

    if(currentSize < (stageSize - SensorRecord::SIZE_RECORD)){
      //room on stage for another record
      if(currentRecordTime > oldestTime && currentRecordTime < youngestTime){
	//record is within time spec
	records.push(currentRecord);
	currentSize += SensorRecord::SIZE_RECORD;
      }

    }
    
    prevRealTimeDelta = realTimeDelta;
    index++;
  }

  /*
  //THIS IS PROBABLY UNECESSARY
  SensorRecord youngestRecord = getRecord(top-1);
  SensorRecord oldestRecord = getRecord(bottom);
  unsigned int youngestRecordTime = youngestRecord.getTimeDelta()*measurementInterval;
  unsigned int oldestRecordTime = oldestRecord.getTimeDelta()*measurementInterval;
  
  if((getCurrentSize() == 0) ||
     (youngestRecordTime < oldestTime) ||
     (oldestRecordTime > youngestTime)){
    return 0;
  }

  
  
  while((oldestRecord.getTimeDelta() * measurementInterval) > oldest
	&& index > bottom){
    index--;
    
  }

  */
}

const uint8_t * SensorStore::package() const {
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
SensorRecord SensorStore::getRecord(int index){
  return store[index];
}

void SensorStore::printStore(){
  std::cout << "\tID\tDELTA\tTEMP\n";
  for (int i = top-1; i >= bottom; i--) {
    store[(i%storeSize)].print();
    std::cout << "\n\r";
  }
}

