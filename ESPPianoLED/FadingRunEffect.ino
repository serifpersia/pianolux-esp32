//FadingRunEffect.cpp
#include "FadingRunEffect.h"
FadingRunEffect ::FadingRunEffect(int effectLen, int startPosition, CHSV splashColor, int headFadeRate, int velocity) {
  // Initialize the effect parameters
  this->effectLen = effectLen;
  this->startPosition = startPosition;
  this->step = 0;
  this->splashColor = splashColor;
  this->headFadeRate = headFadeRate;
  this->lastUpdate = millis();
  this->velocity = velocity;
}

int MAX_VALUE = 255;
int LOWEST_BRIGHTNESS = 50;
int LOWEST_SATURATION = 200;

int HEAD_FADE_LOW_THRESHOLD = 50;
int HEAD_FADE_HI_THRESHOLD = 255;

int FadingRunEffect ::getSaturation(int velocity) {
  return adjustValue(velocity, LOWEST_SATURATION, MAX_VALUE);
}

int FadingRunEffect ::getBrightness(int velocity) {
  return adjustValue(velocity, LOWEST_BRIGHTNESS, MAX_VALUE);
}

int FadingRunEffect ::calcOffset(int step, int velocity) {
  int adjustedVelocity = adjustValue(velocity, HEAD_FADE_LOW_THRESHOLD, HEAD_FADE_HI_THRESHOLD);
  int offset = step * adjustedVelocity / MAX_VELOCITY;
  if (offset > step) {
    return step;
  } else {
    return offset;
  }
}


int FadingRunEffect ::adjustValue(int value, int lowerThreshold, int maxValue) {
  return lowerThreshold + (value * (maxValue - lowerThreshold) / maxValue);
}

void FadingRunEffect ::setHeadLED(int step) {
  if (step > effectLen) {
    return;
  }
  int pos1 = ledNum(this->startPosition + calcOffset(step, velocity));
  int pos2 = ledNum(this->startPosition - calcOffset(step, velocity));
  if (isOnStrip(pos1)) {
    if (splashColor != CHSV(0, 0, 0)) {
      leds[pos1] = CHSV(splashColor.hue, splashColor.saturation, getBrightness(velocity));
    } else {
      leds[pos1] += CHSV(getHueForPos(startPosition), getSaturation(velocity), getBrightness(velocity));
    }
    leds[pos1].fadeToBlackBy(headFadeRate * step);
  }

  if (pos1 != pos2 && isOnStrip(pos2)) {
    if (splashColor != CHSV(0, 0, 0)) {
      leds[pos2] = CHSV(splashColor.hue, splashColor.saturation, getBrightness(velocity));
    } else {
      leds[pos2] += CHSV(getHueForPos(startPosition), getSaturation(velocity), getBrightness(velocity));
    }
    leds[pos2].fadeToBlackBy(headFadeRate * step);
  }
}

// Calculate the color of the LED at a given position
void FadingRunEffect ::nextStep() {
  setHeadLED(this->step);
  this->step++;
}
int FadingRunEffect ::getSteps() {
  return effectLen;
}

boolean FadingRunEffect ::finished() {
  return step > getSteps();
}