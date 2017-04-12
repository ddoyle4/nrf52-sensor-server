#include "RangeFinder.h"

extern Serial pc;

#define abs(X) ((X) < 0 ? -(X) : (X))

RangeFinder::RangeFinder(PinName triggerPin, PinName echoPin)
  : trigger(triggerPin), echo(echoPin),
  Sensor()
{
    lastSample = 0;
    averageSample = 0;
    averageAlpha = 0.05;
    resetThreshold = 0.01 * 5877;
}

RangeFinder::~RangeFinder(void)
{

}

float RangeFinder::updateDistance(void)
{
    unsigned int curPulse = 0;

    while (curPulse < PULSES_PER_SAMPLE)
    {
        echoTimer.reset();

        wait_ms(50);
        trigger = 0;
        wait_us(2);
        trigger = 1;
        wait_us(10);
        trigger = 0;

        while (echo == 0);
        echoTimer.start();
        while (echo == 1);
        echoTimer.stop();
        pulseBuffer[curPulse] = echoTimer.read_us();
        curPulse++;
    }

    unsigned int sum = 0;

    for (int i = 0; i < PULSES_PER_SAMPLE; i++)
    {
        sum += pulseBuffer[i];
    }

    lastSample = sum / PULSES_PER_SAMPLE;

    if (averageSample == 0.0 || abs(averageSample - lastSample) > resetThreshold ) {
        averageSample = lastSample;
    } else {
        averageSample = (averageSample * (1 - averageAlpha)) + (lastSample * averageAlpha);
    }

    
    float val =  (averageSample / 5877) + 0.017; // +17mm adjustment
    lastReading = val;
    return lastReading;
}

float RangeFinder::read(){
  return updateDistance();
}
