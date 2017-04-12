#ifndef __SENSOR_STORE__
#define __SENSOR_STORE__

#include "SensorRecord.h"
//#include "mbed.h"
#include <stdint.h>
#include <cstring>
#include <cmath>
#include <stack>
#include <time.h>

class SensorStore {

 public:
  //size of the stage header
  static const unsigned int STAGE_HEADER_SIZE = 8;
  //size of the repeating record unit (timeDelta + reading value)
  static const unsigned int STAGE_RECORD_UNIT_SIZE = 6;
  
  SensorStore(int memorySize, uint16_t _measurementInterval, float _threshold);
  ~SensorStore();
  int getStoreSize();
  void addReading(float reading);
  unsigned int flush(uint8_t * stage, unsigned int oldest, unsigned int youngest, uint8_t sensorID, int stageSize);
  SensorRecord getRecord(int index);
  void printStore();
  int getCurrentSize();
  float getThreshold(){ return threshold; };
  void setThreshold(float _threshold){ threshold = _threshold; };
  
 private:
  void setStageData(uint8_t *stage,
		    unsigned int start,
		    std::stack<SensorRecord> records,
		    unsigned int lastStageTimeDelta,
		    uint8_t sensorID,
		    bool missingFlag,
		    bool averageCarriedForward
		    );
  unsigned int getOldestRealTimeDelta();
  void formatStage(uint8_t *stage, int size);
  int storeSize, top, bottom, memorySize;
  SensorRecord * store;
  uint16_t measurementInterval;
  float threshold;
  time_t lastReadingTime;
};

#endif
