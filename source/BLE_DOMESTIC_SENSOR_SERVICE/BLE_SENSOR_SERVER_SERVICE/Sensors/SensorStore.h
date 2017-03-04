#ifndef __SENSOR_STORE__
#define __SENSOR_STORE__

#include "SensorRecord.h"
//#include "mbed.h"
#include <stdint.h>
#include <cstring>
#include <stack>
#include <time.h>

class SensorStore {

 public:
  //size of the stage header
  static const unsigned int STAGE_HEADER_SIZE = 8;
  //size of the repeating record unit (timeDelta + reading value)
  static const unsigned int STAGE_RECORD_UNIT_SIZE = 6;
  
  SensorStore(int memorySize, int stageSize, uint16_t _measurementInterval);
  ~SensorStore();
  int getStoreSize();
  void addReading(float reading);
  unsigned int flush(unsigned int oldest, unsigned int youngest, uint8_t sensorID);
  const uint8_t * package() const;
  SensorRecord getRecord(int index);
  void printStore();
  int getCurrentSize();
  int getStageSize();
  
 private:
  void setStageData(unsigned int start,
		    std::stack<SensorRecord> records,
		    unsigned int lastStageTimeDelta,
		    uint8_t sensorID,
		    bool missingFlag
		    );
  unsigned int getOldestRealTimeDelta();
  void formatStage();
  int storeSize, top, bottom, memorySize, stageSize;
  SensorRecord * store;
  uint8_t * stage;
  uint16_t measurementInterval;
  static time_t lastReadingTime;
};






#endif
