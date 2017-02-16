#ifndef __TEMPERATURE_RECORD__
#define __TEMPERATURE_RECORD__

#include <stdint.h>
#include <string.h>

class TemperatureRecord {

 public:

  /* Structure of data bytes 
   * NOTE Should consider reducing ID to ~4 bits, and then have another
   * 4 bits for flags. Don't need 8 bits for ID, it's only used to spot
   * when a reading has been missed
   */
  static const unsigned OFFSET_OF_ID = 0;
  static const unsigned OFFSET_OF_TIME_DELTA = OFFSET_OF_ID + sizeof(uint8_t);
  static const unsigned OFFSET_OF_TEMP = OFFSET_OF_TIME_DELTA + sizeof(unsigned int);
  static const unsigned SIZEOF_RECORD = sizeof(uint8_t) + sizeof(unsigned int) + sizeof(float);
  
  TemperatureRecord(float temp, uint8_t _id, unsigned int _timeDelta);
  
  uint8_t *getData(void);


  const uint8_t *getData(void) const;
  uint8_t getID();
  
  void print();
 private:
  uint8_t bytes[SIZEOF_RECORD];
  float tempReading;
  uint8_t id;
  unsigned int timeDelta;

};

#endif
