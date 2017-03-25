#include "SensorStore.h"
#include <stdlib.h>
#include <iostream>

time_t SensorStore::lastReadingTime = 0;

SensorStore::SensorStore(int _memorySize, uint16_t interval, float _threshold) :
  top(0),
  bottom(0),
  memorySize(_memorySize),
  measurementInterval(interval),
  threshold(_threshold)
{

  /* Size of SensorRecord will determine how many readings can be stored*/
  storeSize = memorySize / (sizeof(SensorRecord));
  
  //IMPORTANT NOTE: look into using mbed_ualloc() instead - https://docs.mbed.com/docs/getting-started-mbed-os/en/latest/Full_Guide/memory/#memory-allocation-in-mbed-os
  store = (SensorRecord *) malloc(sizeof(SensorRecord) * storeSize);
}

void SensorStore::formatStage(uint8_t * stage, int size){
  for(int i = 0; i < size; i++){
    stage[i] = 0x00;
  }
}

SensorStore::~SensorStore(){
  free(store);
}

int SensorStore::getStoreSize(){
  return storeSize;
}

//when adding threshold, consider adding a record even if within
//threshold in situations where ((time_now - lastReadingTime) > (0.75 * max_interval_value))
void SensorStore::addReading(float value){

  int currentSize = getCurrentSize();

  if(currentSize > 0){  // Only add values that exceed threshold relative to previous reading
    float lastReading = store[(top-1)%storeSize].getReading();
    if(std::fabs(lastReading - value) < threshold){
      return;
    }
  }
  
  uint16_t timeDelta = 0;
  time_t currentTime = time(NULL);

  if(currentSize > 0){ // Ignore time delta for first reading
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
 * are prioritized.
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
unsigned int SensorStore::flush(uint8_t *stage, unsigned int oldestTimeDelta, unsigned int youngestTimeDelta, uint8_t sensorID, int stageSize){  
  //check for space - in a perfect world this wouldn't be necessary
  if(stageSize < (STAGE_HEADER_SIZE + SensorRecord::SIZE_RECORD)){ return 0; }
  
  unsigned int
    index = bottom,
    acf_accumulatedTime = 0,
    acf_index,
    relationalDelta,
    realTimeDelta,
    prevRealTimeDelta,
    accumulatedRelTime = 0,
    currentSize = 0,
    maxReadingsSize = stageSize - STAGE_HEADER_SIZE - SensorRecord::SIZE_RECORD;

  bool
    missedData = false,
    acf_enabled = false; //flag - records not flushed due to space limitations - promps further calls
  
  //real time delta of time pointed to by relational delta of store[bottom]
  prevRealTimeDelta = getOldestRealTimeDelta();

  SensorRecord currentRecord;     
  std::stack<SensorRecord> records;
  
  //find all relevant records
  while(index < top){ //TODO fix this - maybe make top and bottom unsigned ints
    currentRecord = getRecord(index%storeSize);

    //amount of time (secs) between this record and previous record
    relationalDelta = currentRecord.getTimeDelta()*measurementInterval;
    
    //amount of time (secs) between now and when this record was stored
    realTimeDelta = prevRealTimeDelta - relationalDelta;

    //is record is within desired period?
    if( (realTimeDelta <= oldestTimeDelta) && (realTimeDelta >= youngestTimeDelta) ){
      if( currentSize <= maxReadingsSize){ //is there space for this record?
	records.push(currentRecord);
	currentSize += SensorRecord::SIZE_RECORD;
      } else {
	missedData = true;
      }
    }

    //keep track of realTime of any record that was taken immediately before
    //defined period - for use if no record is included
    if(records.size() == 0
       && realTimeDelta > oldestTimeDelta){
      acf_accumulatedTime = realTimeDelta;
      acf_index = index;
      acf_enabled = true;
    }
    
    // accumulate timeDelta to last appropriately staged reading
    if( (records.size() > 0) &&                                    
	((realTimeDelta < youngestTimeDelta) ||
	 (currentSize >= maxReadingsSize) )){
      accumulatedRelTime += relationalDelta;
    }

    prevRealTimeDelta = realTimeDelta;
    index++;
  }

  if(acf_enabled && records.size() == 0  && getCurrentSize() > 0){
    /**
     * There are readings in the store, but none fall within the time period specified,
     * because the sensor reading remained within the threshold limit for the 
     * defined period and/or the defined period falls in between two read intervals. Either
     * way, we set the "average carried forward" flag and return the closest older reading
     * which will serve as an average for the client to use for the specified period.
     **/
    records.push(getRecord((top - 1)%storeSize));
    double timeDelta = difftime(time(NULL), lastReadingTime) + 0.5;
    setStageData(stage, oldestTimeDelta, records, (unsigned int)timeDelta, sensorID, false, true);
    
    return 1;
  }

  
  double timeDelta = difftime(time(NULL), lastReadingTime) + accumulatedRelTime + 0.5;
  setStageData(stage, oldestTimeDelta, records, (unsigned int)timeDelta, sensorID, missedData, false);
  
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

  unsigned int relationalTimeDelta = 0;
  unsigned int index = top - 1; 
  for(int i = getCurrentSize(); i > 0; i--){
    relationalTimeDelta += getRecord(index%storeSize).getTimeDelta();
    index--;
  }

  return (relationalTimeDelta * measurementInterval) + timeDelta;
}

void SensorStore::setStageData(uint8_t *stage, unsigned int start, std::stack<SensorRecord> records,
			       unsigned int timeDelta, uint8_t sensorID, bool missing, bool averageCarriedForward){

  int indexOffset = 0, size = records.size();

  //set flags
  uint8_t flagsAndSensorID = 0x00;
  if(missing){ flagsAndSensorID |= 0x80; }
  if(averageCarriedForward){ flagsAndSensorID |= 0x40; }
  
  flagsAndSensorID = flagsAndSensorID | (sensorID & 0x0F);
  stage[indexOffset++] = flagsAndSensorID;

  //set starting time
  std::memcpy(&stage[indexOffset], &timeDelta, 4);
  indexOffset += 4;

  //set count
  stage[indexOffset++] = (uint8_t)((records.size() > 255) ? 0xFF : (records.size() & 0xFF));

  //set measurement interval
  std::memcpy(&stage[indexOffset], &measurementInterval, 2);
  indexOffset += 2;

  for(int i = 0; i < size; i++){
    std::memcpy(&stage[indexOffset], ((SensorRecord)records.top()).getData(), SensorRecord::SIZE_RECORD);
    records.pop();
    indexOffset += SensorRecord::SIZE_RECORD;
  }
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

