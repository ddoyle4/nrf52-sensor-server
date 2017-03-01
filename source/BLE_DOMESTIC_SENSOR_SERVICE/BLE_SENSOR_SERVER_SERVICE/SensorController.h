#ifndef __SENSOR_CONTROLLER_H__
#define __SENSOR_CONTROLLER_H__

#include "Sensors/DS18B20_TemperatureSensor.h"
#include "Sensors/SensorStore.h"
#include "Sensors/Sensor.h"
#include <vector>
#include "mbed.h"

typedef enum sensorType {
  DS18B20_TEMPERATURE,
  RANGE_FINDER,
  PRESSURE
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
  bool addSensor(Sensor *sensor, uint16_t interval, sensorType _type, PinName *pins, int numPins);

 private:
  Serial *debugger;
  EventQueue *eventQueue;
  void performMeasurement(int t);
  sensorControl sensors[NUM_SENSOR_SLOTS];
  int numActiveSensors;
};

#endif



