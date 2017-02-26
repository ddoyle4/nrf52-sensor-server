#include "SensorController.h"

SensorController::SensorController() :
  eventQueue(EventQueue(32*EVENTS_EVENT_SIZE)),
  numActiveSensors(0)
{}

SensorController::~SensorController(){}

void SensorController::performMeasurement(int t){
  sensors[0].eventID = 0;
}

bool SensorController::addSensor(Sensor _sensor, uint16_t interval, sensorType _type, PinName *_pins, int numPins){
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
  int id = eventQueue.call_every(ms_interval, this, &SensorController::performMeasurement, numActiveSensors);

  sensors[numActiveSensors].eventID = id;

  numActiveSensors++;

  return true;
}








