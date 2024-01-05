//FadeController.cpp
#include "FadeController.h"
FadeController ::FadeController() {
}

// Set the duration of the transition and the number of steps
unsigned long transitionTime = 1000;  // in milliseconds
uint8_t steps = 100;
uint8_t BG_FADE_OFFSET = 0;

void FadeController::fade(uint8_t fadeRate) {
  boolean bgModeOn = (bgColor != CRGB(0));
  boolean guideModeOn = (guideColor != CRGB(0));

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    uint8_t effectiveSplashRate = fadeRate;
    if (keysOn[i]) {
      if (MODE == COMMAND_SPLASH)
        effectiveSplashRate = 1;
      else
        effectiveSplashRate = 0;
    }
    uint8_t ledNo = ledNum(i);
    CRGB currentColor = leds[ledNo];

    // Trigger is complete, fade the LED
    if (effectiveSplashRate > 0) {
      if (bgModeOn) {
        // Only background mode is on
        if (leds[ledNo] != bgColor) {
          nblend(leds[ledNo], bgColor, effectiveSplashRate + BG_FADE_OFFSET);
          if (distance(leds[ledNo], bgColor) < 5) {
            leds[ledNo] = bgColor;
          }
        }
      } else if (guideModeOn) {
        // Only guide mode is on
        if (leds[ledNo] != bgColor) {
          // Don't fade LEDs matching guideColor
          leds[ledNo] = guideColor;
        }
      } else {
        // Both background mode and guide mode are off, fade to black
        leds[ledNo].fadeToBlackBy(fadeRate);
      }
    }
  }
}
