#include "SensorController.h"

SensorController::SensorController(EventQueue *queue, int _stageSize) :
  //  eventQueue(EventQueue(32*EVENTS_EVENT_SIZE)),
  eventQueue(queue),
  numActiveSensors(0),
  currentStoreAllocation(0),
  stageSize(_stageSize),
  lastStartTimeUpdate(0)
{
  //lookup mbed memory allocation for this
  stage = (uint8_t *)malloc(sizeof(uint8_t)*stageSize);
}

SensorController::~SensorController(){
  free(stage);
}

void SensorController::performMeasurement(int t){
  //  sensors[0].eventID = 0;
  sensorControl sensor = sensors[t];
  float reading = sensor.sensor->read();
  sensor.store->addReading(reading);
}

uint16_t SensorController::getMaxBufferSize(){
  return MAX_STORE_ALLOCATION/NUM_SENSOR_SLOTS;
}

int SensorController::addSensor(Sensor *_sensor, uint16_t interval, float threshold, sensorType _type, PinName *_pins, int numPins, int memSize){
  if( ((memSize + currentStoreAllocation) > MAX_STORE_ALLOCATION) || numActiveSensors >= NUM_SENSOR_SLOTS){
    return -1;
  }

  currentStoreAllocation += memSize;
  
  sensorControl newSensor;
  newSensor.sensor = _sensor;
  newSensor.measurementInterval = interval;
  newSensor.type = _type;

  newSensor.store = new SensorStore(memSize, interval, threshold);
  newSensor.pins = _pins;
  newSensor.numAllocatedPins = numPins;
  newSensor.eventID = 0; //this is set correctly below

  sensors[numActiveSensors] = newSensor;

  //TODO add checking here - could become negative and nasty
  int ms_interval = (int)(interval*1000);
  int id = eventQueue->call_every(ms_interval, this, &SensorController::performMeasurement, numActiveSensors);

  sensors[numActiveSensors].eventID = id;

  numActiveSensors++;
  
  return (numActiveSensors - 1);
}

/** 
 * Flushes the sensor store of the sensor and creates 
 * the appropriate header for the data in memory.
 * 
 * @param oldLimit oldest time limit to include records
 * @param youngLimit youngest time limit to include records
 * @param sensor index of the sensor
 * 
 * @return size of internal stage that was flushed 
 */
unsigned int SensorController::flushSensorStore(unsigned int oldLimit, unsigned int youngLimit, uint8_t sensor, command_type ctype){
  SensorStore * store = sensors[sensor].store;
  lastStartTimeUpdate = time(NULL);
  unsigned int numRecords = store->flush(stage, oldLimit, youngLimit, sensor, stageSize, ctype);
  return (numRecords * SensorStore::STAGE_RECORD_UNIT_SIZE) + SensorStore::STAGE_HEADER_SIZE;
}

const uint8_t * SensorController::getPackage() const{
  const uint8_t * packaged = stage;
  return packaged;
}

void SensorController::updateStageStartTime(){
  unsigned int startTime;
  std::memcpy(&startTime, &stage[STAGE_START_TIME_OFFSET], sizeof(unsigned int));
  double elapsedTime = difftime(time(NULL), lastStartTimeUpdate);
  unsigned int newStartTime = startTime + (unsigned int)(elapsedTime + 0.5);
  std::memcpy(&stage[STAGE_START_TIME_OFFSET], &newStartTime, sizeof(unsigned int));
  lastStartTimeUpdate = time(NULL);
}

void SensorController::writeErrorCode(error_code code){

  //identify that error has occured with alternating
  //patterns of 1/0s
  for(int i=0; i<ERROR_CODE_FLAG_REPETITION; i++){
    stage[i] = ERROR_CODE_FLAG;
  }

  switch(code){
  case UNRECOGNISED_COMMAND:
    stage[ERROR_CODE_FLAG_REPETITION] = 0x00;
    break;
  default:
    break;
  }

}


void SensorController::updateGapBufferData(uint8_t *data){

  float currentSize, maxSize;
  uint8_t bitVal;
  SensorStore * store;

  //clear current values
  for(int i=0; i < 4; i++){
    data[i] = 0x00;
  }
  
  for(int i = 0; i < numActiveSensors; i++){
    store = sensors[i].store;
    currentSize = (float)store->getCurrentSize();
    maxSize = (float)store->getStoreSize();
    bitVal = (uint8_t)( ((currentSize / maxSize) * 16.0) + 0.5 );

    if(i%2 == 0){ bitVal <<= 4; } 
    data[i] |= bitVal;
  }
}


