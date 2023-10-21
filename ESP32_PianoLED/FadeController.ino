//FadeController.cpp
#include "FadeController.h"
FadeController ::FadeController() {
}

// Set the duration of the transition and the number of steps
int transitionTime = 1000;  // in milliseconds
int steps = 100;
int BG_FADE_OFFSET = 0;

void FadeController::fade(int fadeRate) {
  boolean bgModeOn = (bgColor != CRGB(0));
  boolean guideModeOn = (guideColor != CRGB(0));

  for (int i = 0; i < NUM_LEDS; i++) {
    int effectiveSplashRate = fadeRate;
    if (keysOn[i]) {
      if (MODE == COMMAND_SPLASH)
        effectiveSplashRate = 1;
      else
        effectiveSplashRate = 0;
    }
    int ledNo = ledNum(i);
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