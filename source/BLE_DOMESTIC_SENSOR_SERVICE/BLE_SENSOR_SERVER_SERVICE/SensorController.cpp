#include "SensorController.h"

SensorController::SensorController(Serial *debug, EventQueue *queue) :
  debugger(debug),
  //  eventQueue(EventQueue(32*EVENTS_EVENT_SIZE)),
  eventQueue(queue),
  numActiveSensors(0)
{}

SensorController::~SensorController(){}

void SensorController::performMeasurement(int t){
  //  debugger->printf("Performing measurement for: %d", t);
  //  sensors[0].eventID = 0;
  sensorControl sensor = sensors[t];
  float reading = sensor.sensor->read();
  sensor.store->addReading(reading);
  //debugger->printf("READING:%f. STORE SIZE: %d\n\r", reading, sensor.store->getStoreSize());
}

uint16_t SensorController::getMaxBufferSize(){
  return MAX_STORE_ALLOCATION/NUM_SENSOR_SLOTS;
}

int SensorController::addSensor(Sensor *_sensor, uint16_t interval, sensorType _type, PinName *_pins, int numPins){
  if(numActiveSensors > NUM_SENSOR_SLOTS){
    return -1;
  }
  
  sensorControl newSensor;
  newSensor.sensor = _sensor;
  newSensor.measurementInterval = interval;
  newSensor.type = _type;
  newSensor.store = new SensorStore((MAX_STORE_ALLOCATION/NUM_SENSOR_SLOTS), 512, interval);
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
unsigned int SensorController::flushSensorStore(unsigned int oldLimit, unsigned int youngLimit, uint8_t sensor){
  SensorStore * store = sensors[sensor].store;
  unsigned int numRecords = store->flush(oldLimit, youngLimit, sensor);
  return (numRecords * SensorStore::STAGE_RECORD_UNIT_SIZE) + SensorStore::STAGE_HEADER_SIZE;
}

const uint8_t * SensorController::getPackage(uint8_t sensor) const{
  return sensors[sensor].store->package();
}








