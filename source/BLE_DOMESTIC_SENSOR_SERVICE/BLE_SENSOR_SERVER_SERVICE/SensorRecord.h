#ifndef __SENSOR_RECORD__
#define __SENSOR_RECORD__

#include <stdint.h>
#include <string.h>

class SensorRecord {

 public:

  static const unsigned OFFSET_TIME_DELTA = 0;
  static const unsigned SIZE_TIME_DELTA = 2;
  static const unsigned OFFSET_OF_VALUE = sizeof(uint8_t) * SIZE_TIME_DELTA;
  static const unsigned SIZE_VALUE = 4;
  static const unsigned SIZE_RECORD = SIZE_VALUE + SIZE_TIME_DELTA;

  SensorRecord(float reading, uint16_t timeDelta);
  uint16_t getTimeDelta();
  uint8_t *getData(void);
  const uint8_t *getData(void) const;
  
  void print();
 private:
  uint8_t bytes[SIZE_RECORD];
  float reading;
  uint16_t timeDelta;

};

#endif
