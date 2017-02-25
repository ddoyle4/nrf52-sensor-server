
#include "SensorStore.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <stdio.h>


void printStoreHex(const uint8_t *store, int size){

  printf("%x %x %x %x\n", store[0], store[1], store[2], store[3]);

  int index = 4;
  
  for (int i = 0; i < size; i++){
    for (int j = 0; j < SensorRecord::SIZE_RECORD; j++){
      printf("%x ", store[index++]);
    }
    std::cout << "\n";
    }
}


int main(){

  int memSize = 1024, stageSize = (512-12);
  uint16_t interval = 2;
  
  SensorStore testStore = SensorStore(memSize, stageSize, interval);
  std::cout << "testStore mem size: " << testStore.getStoreSize() << std::endl;
  std::cout << "testStore stage size: " << testStore.getStageSize() << std::endl;


  
  time_t timeBeforeAdding = time(NULL);

  testStore.addReading(10.0);
  sleep(2*interval);                // + 4
  testStore.addReading(11.0);       
  sleep(3*interval);                // + 6
  testStore.addReading(12.0);

  sleep(2*interval);
  time_t timeAfterAdding = time(NULL);

  std::cout << "size after adding: " << testStore.getCurrentSize() << std::endl;

  std::cout << "Store: " << std::endl;
  testStore.printStore();


  std::cout << "time before adding: " << timeBeforeAdding << std::endl;
  std::cout << "time after adding: " << timeAfterAdding << std::endl;

  unsigned int oldest = (unsigned int)difftime(time(NULL), timeBeforeAdding);
  std::cout << "oldest: " << (oldest) << std::endl;       // 14
  unsigned int youngest = (unsigned int)difftime(time(NULL), timeAfterAdding);
  std::cout << "youngest: " << (youngest) << std::endl; // 0

  unsigned int num = testStore.flush(oldest, youngest);
  std::cout << "numbder of records staged: " << num << std::endl;
  printStoreHex(testStore.package(), num);

  
  
  
  return 0;
}
