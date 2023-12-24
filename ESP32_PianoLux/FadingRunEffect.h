//FadingRunEffect.h
#ifndef FadingRunEffect_h
#define FadingRunEffect_h

#include <Arduino.h>

class FadingRunEffect {

  public:
    FadingRunEffect(int effectLen, int startPosition, CHSV splashColor, int headFadeRate, int velocity);
    void nextStep();
    boolean finished();
    int getSaturation(int velocity);
    int getBrightness(int velocity);
    void setHeadLED(int step);
    int getSteps();
    int adjustValue(int value, int lowerThreshold, int maxValue);
    int calcOffset(int step, int velocity);

  private:
    int effectLen;
    int startPosition;
    int step;
    CHSV splashColor;
    int headFadeRate;
    int velocity;
    unsigned long lastUpdate;
};

#endif
