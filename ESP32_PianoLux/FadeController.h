//FadeController.h
#ifndef FadeController_h
#define FadeController_h

#include <Arduino.h>

class FadeController {

  public:
    FadeController();
    void fade(uint8_t fadeRate);
};

#endif
