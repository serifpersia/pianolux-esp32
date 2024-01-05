//FadingRunEffect.cpp
#include "FadingRunEffect.h"
FadingRunEffect ::FadingRunEffect(uint8_t effectLen, uint8_t startPosition, CHSV splashColor, uint8_t headFadeRate, uint8_t velocity) {
  // Initialize the effect parameters
  this->effectLen = effectLen;
  this->startPosition = startPosition;
  this->step = 0;
  this->splashColor = splashColor;
  this->headFadeRate = headFadeRate;
  this->lastUpdate = millis();
  this->velocity = velocity;
}

uint8_t MAX_VALUE = 255;
uint8_t LOWEST_BRIGHTNESS = 50;
uint8_t LOWEST_SATURATION = 200;

uint8_t HEAD_FADE_LOW_THRESHOLD = 50;
uint8_t HEAD_FADE_HI_THRESHOLD = 255;

uint8_t FadingRunEffect ::getSaturation(uint8_t velocity) {
  return adjustValue(velocity, LOWEST_SATURATION, MAX_VALUE);
}

uint8_t FadingRunEffect ::getBrightness(uint8_t velocity) {
  return adjustValue(velocity, LOWEST_BRIGHTNESS, MAX_VALUE);
}

uint8_t FadingRunEffect ::calcOffset(uint8_t step, uint8_t velocity) {
  uint8_t adjustedVelocity = adjustValue(velocity, HEAD_FADE_LOW_THRESHOLD, HEAD_FADE_HI_THRESHOLD);
  uint8_t offset = step * adjustedVelocity / MAX_VELOCITY;
  if (offset > step) {
    return step;
  } else {
    return offset;
  }
}


uint8_t FadingRunEffect ::adjustValue(uint8_t value, uint8_t lowerThreshold, uint8_t maxValue) {
  return lowerThreshold + (value * (maxValue - lowerThreshold) / maxValue);
}

void FadingRunEffect ::setHeadLED(uint8_t step) {
  if (step > effectLen) {
    return;
  }
  uint8_t pos1 = ledNum(this->startPosition + calcOffset(step, velocity));
  uint8_t pos2 = ledNum(this->startPosition - calcOffset(step, velocity));
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
uint8_t FadingRunEffect ::getSteps() {
  return effectLen;
}

boolean FadingRunEffect ::finished() {
  return step > getSteps();
}
