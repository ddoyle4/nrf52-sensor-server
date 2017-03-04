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
  debugger->printf("READING:%f. STORE SIZE: %d\n\r", reading, sensor.store->getStoreSize());
}

bool SensorController::addSensor(Sensor *_sensor, uint16_t interval, sensorType _type, PinName *_pins, int numPins){
  if(numActiveSensors > NUM_SENSOR_SLOTS){
    return false;
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
  return true;
}

uint8_t * SensorController::flushSensorStore(unsgined int oldLimit, unsigned int youngLimit, uint8_t sensor){
  unsigned int numFlushed = sensors[sensor].store->flush(oldLimit, youngLimit, sensor);

}








