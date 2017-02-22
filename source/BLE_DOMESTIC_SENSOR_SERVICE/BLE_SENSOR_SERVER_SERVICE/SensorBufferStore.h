#ifndef __SENSOR_STORE__
#define __SENSOR_STORE__

#include "SensorRecord.h"
#include "mbed.h"
#include <stdint.h>
#include <cstring>
#include <stack>

class SensorStore {

 public:
  static const unsigned STAGE_HEADER_SIZE = 4;
  SensorStore(int memorySize, int stageSize, uint16_t _measurementInterval);
  ~SensorStore();
  int getStoreSize();
  void addReading(float reading);
  unsigned int flush(unsigned int oldest, unsigned int youngest);
  const uint8_t * package() const;
  SensorRecord getRecord(int index);
  void printStore();
  int getCurrentSize();
  int getStageSize();
  
 private:
  uint8_t getNextID();
  void formatStage();
  unsigned  int storeSize, top, bottom, memorySize, stageSize;
  SensorRecord * store;
  uint8_t * stage;
  uint16_t measurementInterval;
  static time_t lastReadingTime;
};






#endif
