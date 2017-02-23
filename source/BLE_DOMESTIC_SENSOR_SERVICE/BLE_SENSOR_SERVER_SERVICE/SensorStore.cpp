#include "SensorStore.h"
#include <stdlib.h>
#include <iostream>

time_t SensorStore::lastReadingTime = 0;

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

//when adding threshold, consider adding a record even if within
//threshold in situations where ((time_now - lastReadingTime) > (0.75 * max_interval_value))
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
unsigned int SensorStore::flush(unsigned int oldestTimeDelta, unsigned int youngestTimeDelta){
  unsigned int
    index = bottom,
    relationalDelta,
    realTimeDelta,
    prevRealTimeDelta,
    accumulatedTime,
    currentSize = 0,
    maxStageSize = stageSize - STAGE_HEADER_SIZE - SensorRecord::SIZE_RECORD + 1;

  //the oldest time on record for this buffer. It is the time pointed to
  //by the time delta of oldest record in the store
  prevRealTimeDelta = getOldestRealTimeDelta();
  std::cout << "prev readl time delta: " << prevRealTimeDelta << std::endl;
  SensorRecord currentRecord;     

  std::stack<SensorRecord> records;

  bool accumulateTime = false;


  //find all relevant records
  while(index < top){
    currentRecord = getRecord(index%storeSize);

    //amount of time (secs) between this record and previous record
    relationalDelta = currentRecord.getTimeDelta()*measurementInterval;

    //amount of time (secs) between now and when this record was stored
    realTimeDelta = relationalDelta + prevRealTimeDelta;

    //stage record if there is space and it falls within time period
    if( (currentSize < (stageSize - maxStageSize)) &&
	(realTimeDelta <= oldestTimeDelta) &&
	(realTimeDelta >= youngestTimeDelta) ){

      records.push(currentRecord);
      currentSize += SensorRecord::SIZE_RECORD;
    }

    // accumulate timeDelta to last appropriately staged reading
    if( (records.size() > 0) &&                                    
	((realTimeDelta > youngestTimeDelta) ||
	 (currentSize >= (stageSize - maxStageSize - 1))) ){
      accumulatedTime += relationalDelta;
    }

    prevRealTimeDelta = realTimeDelta;
    index++;
  }

  double timeDelta = difftime(time(NULL), lastReadingTime) + accumulatedTime + 0.5;
  setStageData(records, (unsigned int)timeDelta);
  return records.size();
}

/** 
 * Returns the time delta for the time pointed to by the time delta of the 
 * oldest record in the store. This will be:
 * current_time - time_of_last_record - last_record_time_delta
 * 
 * 
 * @return oldest time on record for this store
 */
unsigned int SensorStore::getOldestRealTimeDelta(){

  unsigned int timeDelta = (unsigned int)(difftime(time(NULL), lastReadingTime) + 0.5);

  unsigned int index = top - 1; 
  for(int i = getCurrentSize(); i > 0; i--){
    timeDelta += getRecord(index%storeSize).getTimeDelta();
    index--;
  }

  return timeDelta * measurementInterval;
}

void SensorStore::setStageData(std::stack<SensorRecord> records, unsigned int timeDelta){
  int indexOffset = 0;
  std::memcpy(&stage[indexOffset], &timeDelta, 4);
  indexOffset += 4;
  for(int i = 0; i < records.size(); i++){
    std::memcpy(&stage[indexOffset], ((SensorRecord)records.top()).getData(), SensorRecord::SIZE_RECORD);
    records.pop();
  }
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
  std::cout << "\tDELTA\tTEMP\n";
  for (int i = top-1; i >= bottom; i--) {
    store[(i%storeSize)].print();
    std::cout << "\n\r";
  }
}

