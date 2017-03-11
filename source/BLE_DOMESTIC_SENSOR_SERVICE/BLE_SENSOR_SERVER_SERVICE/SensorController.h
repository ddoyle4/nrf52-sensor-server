/*
 * Sensor Controller
 * Used to manage multiple sensors and their associated stores.
 * There is a max number of sensor slots and the associated store
 * of each sensor slot is given an equal share of the memory made
 * available to this sensor controller.
 *
 * To get data from a sensor requires two steps:
 *
 *   step 1: call this.flushSensorStore(oldestTimeDelta, youngestTimeDelta, sensorIndex)
 *           this will write all of the store's values to an internal stage, including header data,
 *           and return the number of records that were written.
 *   step 2: call this.getPackage(sensorIndex) this will return a pointer to that area in memory
 *           containing the sensor's internal stage.
 * 
 *  TODO: perhaps better to just return a struct containing the number of records and a pointer to data.
 */

#ifndef __SENSOR_CONTROLLER_H__
#define __SENSOR_CONTROLLER_H__

#include "Sensors/DS18B20_TemperatureSensor.h"
#include "Sensors/SensorStore.h"
#include "Sensors/Sensor.h"
#include <vector>
#include "mbed.h"

typedef enum sensorType {
  TEMPERATURE,
  RANGE_FINDER,
  PRESSURE,
  NOT_A_SENSOR = 7
} sensorType;

typedef struct sensorControl {
  Sensor *sensor;
  uint16_t measurementInterval;
  sensorType type;
  SensorStore *store;
  int numAllocatedPins;
  PinName * pins;
  int eventID;  //id for callback in event queue
} sensorControl;

class SensorController {

 public:
  static const int NUM_SENSOR_SLOTS = 8;
  static const int MAX_STORE_ALLOCATION = 2560;
  SensorController(Serial *debug, EventQueue *eventQueue);
  ~SensorController();
  
  int addSensor(Sensor *sensor, uint16_t interval, sensorType _type, PinName *pins, int numPins);
  int getNumSensors(){ return numActiveSensors; }
  Sensor * getSensor(int sensorID){ return sensors[sensorID].sensor; }
  unsigned int flushSensorStore(unsigned int oldestLimit, unsigned int youngestLimit, uint8_t sensor);
  const uint8_t * getPackage(uint8_t sensor) const;
  uint16_t getMaxBufferSize();
  
 private:
  Serial *debugger;
  EventQueue *eventQueue;
  void performMeasurement(int t);
  sensorControl sensors[NUM_SENSOR_SLOTS];
  int numActiveSensors;
};

#endif



