//FadingRunEffect.h
#ifndef FadingRunEffect_h
#define FadingRunEffect_h

#include <Arduino.h>

class FadingRunEffect {

  public:
    FadingRunEffect(uint8_t effectLen, uint8_t startPosition, CHSV splashColor, uint8_t headFadeRate, uint8_t velocity);
    void nextStep();
    boolean finished();
    uint8_t getSaturation(uint8_t velocity);
    uint8_t getBrightness(uint8_t velocity);
    void setHeadLED(uint8_t step);
    uint8_t getSteps();
    uint8_t adjustValue(uint8_t value, uint8_t lowerThreshold, uint8_t maxValue);
    uint8_t calcOffset(uint8_t step, uint8_t velocity);

  private:
    uint8_t effectLen;
    uint8_t startPosition;
    uint8_t step;
    CHSV splashColor;
    uint8_t headFadeRate;
    uint8_t velocity;
    unsigned long lastUpdate;
};

#endif
