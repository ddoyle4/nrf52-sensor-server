
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


  time_t daveTime = time(NULL);

  sleep(2*interval);
  
  time_t timeBeforeAdding = time(NULL);

  sleep(2);

  testStore.addReading(10.0);
  sleep(2*interval);                // + 4
  testStore.addReading(11.0);       
  sleep(3*interval);                // + 6
  testStore.addReading(12.0);
  sleep(2*interval);                // + 4
  
  time_t timeAfterAdding = time(NULL);

  std::cout << "size after adding: " << testStore.getCurrentSize() << std::endl;

  std::cout << "Store: " << std::endl;
  testStore.printStore();


  std::cout << "time before adding: " << timeBeforeAdding << std::endl;
  std::cout << "time after adding: " << timeAfterAdding << std::endl;

  unsigned int oldest = (unsigned int)difftime(time(NULL), daveTime);
  std::cout << "oldest: " << (oldest) << std::endl;       // 14
  unsigned int youngest = (unsigned int)difftime(time(NULL), timeBeforeAdding);
  std::cout << "youngest: " << (youngest) << std::endl; // 0
  
  std::cout << "numbder of records staged: " << testStore.flush(oldest, youngest) << std::endl;
  

  
  
  
  return 0;
}
