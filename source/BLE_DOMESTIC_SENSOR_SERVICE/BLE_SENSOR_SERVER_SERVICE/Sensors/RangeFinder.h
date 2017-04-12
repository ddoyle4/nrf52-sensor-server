#include "mbed.h"
#include "Sensor.h"

#define PULSES_PER_SAMPLE 4

class RangeFinder : public Sensor{
public:
    RangeFinder(PinName triggerPin, PinName echoPin);
    ~RangeFinder(void);

    float updateDistance(void);
    float read(void);

protected:
    DigitalOut trigger;
    DigitalIn echo;

    Timer echoTimer;

    unsigned int numPulses;
    unsigned int pulseBuffer[PULSES_PER_SAMPLE];
    unsigned int lastSample;
    float averageSample;
    float averageAlpha;
    float resetThreshold;
};
