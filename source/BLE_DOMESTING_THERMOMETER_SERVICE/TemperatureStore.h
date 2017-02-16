#ifndef __TEMPERATURE_STORE__
#define __TEMPERATURE_STORE__

#include "TemperatureRecord.h"
#include <stdint.h>
#include <cstring>

class TemperatureStore {

 public:
  static const unsigned HEADER_SIZE = 2;
  TemperatureStore(int memorySize);
  ~TemperatureStore();
  int getStoreSize();
  void addReading(float temp);
  void flush();
  const uint8_t * package() const;
  TemperatureRecord getRecord(int index);
  void printStore();
  int getCurrentSize();
  
 private:
  uint8_t getNextID();
  void formatStage();
  int storeSize, top, bottom, memorySize;
  TemperatureRecord * store;
  uint8_t * stage;
};






#endif
