#include "SensorRecord.h"
#include <iostream>

SensorRecord::SensorRecord(float _reading, uint16_t _timeDelta) :
  reading(_reading),
  timeDelta(_timeDelta)
{
  memcpy(&bytes[OFFSET_TIME_DELTA], &_timeDelta, sizeof(uint16_t));
  memcpy(&bytes[OFFSET_OF_VALUE], &_reading, sizeof(float));
}

uint8_t *SensorRecord::getData(){
  return bytes;
}

const uint8_t *SensorRecord::getData() const {
  return bytes;
}

 
void SensorRecord::print(){
  std::cout << "RECORD:\t" << timeDelta << "\t" << reading;
}

