#include "SensorStore.h"
#include <iostream>
#include <time.h>
#include <unistd.h>

int main(){

  int memSize = 1024, stageSize = (512-12);
  uint16_t interval = 2;
  
  SensorStore testStore = SensorStore(memSize, stageSize, interval);
  std::cout << "testStore mem size: " << testStore.getStoreSize() << std::endl;
  std::cout << "testStore stage size: " << testStore.getStageSize() << std::endl;

  time_t timeBeforeAdding = time(NULL);
  std::cout << "size before adding: " << testStore.getCurrentSize() << std::endl;
  testStore.addReading(10.0);
  sleep(2*interval);
  testStore.addReading(11.0);
  sleep(3*interval);
  testStore.addReading(12.0);

  std::cout << "size after adding: " << testStore.getCurrentSize() << std::endl;

  std::cout << "Store: " << std::endl;
  testStore.printStore();

  time_t timeAfterAdding = time(NULL);

  unsigned int oldest = (unsigned int)difftime(time(NULL), timeBeforeAdding);
  unsigned int youngest = (unsigned int)difftime(time(NULL), timeAfterAdding);

  std::cout << "numbder of records staged: " << testStore.flush(oldest, youngest) << std::endl;
  

  
  
  
  return 0;
}
