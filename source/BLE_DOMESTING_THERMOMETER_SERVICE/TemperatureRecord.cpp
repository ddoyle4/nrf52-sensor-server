#include "TemperatureRecord.h"
#include <iostream>

/** 
 * Create a new temperature record and a uint8_t array ready to be
 * sent via BLE.
 * 
 * @param temp The actual temperature reading
 * @param _id  The id of this reading.
 * @param _timeDelta The amount of time in ms that has passed since the previous reading.
 */
TemperatureRecord::TemperatureRecord(float temp, uint8_t _id, unsigned int _timeDelta) :
  tempReading(temp),
  id(_id),
  timeDelta(_timeDelta)
{

  //build the data bytes
  bytes[OFFSET_OF_ID] = _id;
  memcpy(&bytes[OFFSET_OF_TIME_DELTA], &_timeDelta, sizeof(unsigned int));
  memcpy(&bytes[OFFSET_OF_TEMP], &temp, sizeof(float));
}

/** 
 * Returns the data as a uint8_t array.
 * 
 * 
 * @return uint8_t *
 */
uint8_t *TemperatureRecord::getData(){
  return bytes;
}

/** 
 * Returns the data as a const uint8_t array.
 * 
 * 
 * @return const uint8_t *
 */
const uint8_t *TemperatureRecord::getData() const {
  return bytes;
}

 
void TemperatureRecord::print(){
  std::cout << "RECORD:\t" << (int) id << "\t" << timeDelta << "\t" << tempReading;
}

uint8_t TemperatureRecord::getID(){ return id; }
