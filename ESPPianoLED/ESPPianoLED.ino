/*
  PianoLED on ESP32S2/S3 boards is an open-source project that aims to provide MIDI-based LED control to the masses.
  It is developed by a one-person team, yours truly, known as serifpersia, or Scarlett.

  If you modify this code and redistribute the PianoLED project, please ensure that you
  don't remove this disclaimer or appropriately credit the original author of the project
  by linking to the project's source on GitHub: github.com/serifpersia/pianoled-esp32/
  Failure to comply with these terms would constitute a violation of the project's
  MIT license under which PianoLED is released.

  Copyright © 2023 Serif Rami, also known as serifpersia.

*/

//PianoLED

//WIFI Libs
#include <WiFiManager.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

//FastLED Lib
#include <FastLED.h>
#include "FadingRunEffect.h"
#include "FadeController.h"

//USB Lib
#include <usb/usb_host.h>
#include "usbhhelp.hpp"

//ESP Storage Lib
#include <SPIFFS.h>

//MIDI
//#define MIDIOUTTEST 1
#if MIDIOUTTEST
#include <elapsedMillis.h>
elapsedMillis MIDIOutTimer;
#endif


// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

// Constants for LED strip
#define UPDATES_PER_SECOND 60
#define MAX_NUM_LEDS 176         // How many LEDs do you want to control
#define DATA_PIN 18              // Your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  // Define current limit
#define MAX_EFFECTS 128

FadingRunEffect* effects[MAX_EFFECTS];
FadeController* fadeCtrl = new FadeController();


int DEFAULT_BRIGHTNESS = 255;
int NUM_LEDS = 176;       // How many LEDs you want to control
int STRIP_DIRECTION = 0;  // 0 - left-to-right

int NOTES = 12;
int generalFadeRate = 255;
int numEffects = 0;

int lowestNote = 21;    // MIDI note A0
int highestNote = 108;  // MIDI note C8 (adjust as needed)
int useFix;
int pianoScaleRatio;

int getHueForPos(int pos) {
  return pos * 255 / NUM_LEDS;
}
int getNote(int key) {
  return key % NOTES;
}

int ledNum(int i) {
  return STRIP_DIRECTION == 0 ? i : NUM_LEDS - i;
}

int getRandomHue() {
  return random(256);
}

CRGB leds[MAX_NUM_LEDS];
CRGB bgColor = CRGB::Black;
CRGB guideColor = CRGB::Black;

boolean bgOn = false;
boolean keysOn[MAX_NUM_LEDS];

boolean isOnStrip(int pos) {
  return pos >= 0 && pos < NUM_LEDS;
}

uint8_t hue = 0;
uint8_t brightness = 255;
uint8_t bgBrightness = 128;
uint8_t saturation = 255;

uint8_t Slider1Value = 0;
uint8_t Slider2Value = 255;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long previousFadeTime = 0;

unsigned long interval = 20;      // General refresh interval in milliseconds
unsigned long fadeInterval = 20;  // General fade interval in milliseconds

const int MAX_VELOCITY = 128;

const int COMMAND_SET_COLOR = 255;
const int COMMAND_FADE_RATE = 254;
const int COMMAND_ANIMATION = 253;
const int COMMAND_BLACKOUT = 252;
const int COMMAND_SPLASH = 251;
const int COMMAND_SET_BRIGHTNESS = 250;
const int COMMAND_KEY_OFF = 249;
const int COMMAND_SPLASH_MAX_LENGTH = 248;
const int COMMAND_SET_BG = 247;
const int COMMAND_VELOCITY = 246;
const int COMMAND_STRIP_DIRECTION = 245;
const int COMMAND_SET_GUIDE = 244;
const int COMMAND_SET_LED_VISUALIZER = 243;

int MODE = COMMAND_SET_COLOR;

int serverMode;

int animationIndex;

int splashMaxLength = 8;
int SPLASH_HEAD_FADE_RATE = 5;

int clientHueVal;
int clientSaturationVal = 255;
int clientBrightnessVal = 255;
int clientFadeVal;



CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

float distance(CRGB color1, CRGB color2) {
  return sqrt(pow(color1.r - color2.r, 2) + pow(color1.g - color2.g, 2) + pow(color1.b - color2.b, 2));
}

void StartupAnimation() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(getHueForPos(i), 255, 255);
    FastLED.show();
    leds[i] = CHSV(0, 0, 0);
  }
  FastLED.show();
}

void setup() {

  Serial.begin(115200);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);         // GRB ordering
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);  // set power limit
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

  StartupAnimation();

  WiFiManager wm;
  bool res = wm.autoConnect("PianoLED AP", "pianoled99");

  if (!res) {
    Serial.println("Failed to connect");
    // Take action if the connection fails, e.g., restart the ESP
    // ESP.restart();
  }
  else
  {
    Serial.println("Connected...yeey :)");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // Initialize and start mDNS
  if (MDNS.begin("pianoled")) {
    Serial.println("MDNS Responder Started!");
  }

  // Serve HTML from ESP32 SPIFFS data directory
  if (SPIFFS.begin()) {
    server.serveStatic("/", SPIFFS, "/");
    server.onNotFound([](AsyncWebServerRequest * request) {
      if (request->url() == "/") {
        request->send(SPIFFS, "/index.html", "text/html");
      } else {
        request->send(404, "text/plain", "Not Found");
      }
    });
  } else {
    Serial.println("Failed to mount SPIFFS file system");
  }

  AsyncElegantOTA.begin(&server);

  server.begin();

  // Add service to mDNS for HTTP
  MDNS.addService("http", "tcp", 80);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  usbh_setup(show_config_desc_full);  //init usb host for midi devices

  setIPLeds();
}

void loop() {

  AsyncElegantOTA.loop();

  webSocket.loop();  // Update function for the webSockets

  usbh_task();

  currentTime = millis();

#ifdef MIDIOUTTEST
  if (isMIDIReady && (MIDIOutTimer > 1000)) {
    ESP_LOGI("", "MIDI send 4 bytes");
    MIDIOut->num_bytes = 4;
    memcpy(MIDIOut->data_buffer, "\x09\x90\x3c\x7a", 4);
    err = usb_host_transfer_submit(MIDIOut);
    if (err != ESP_OK) {
      ESP_LOGI("", "usb_host_transfer_submit Out fail: %x", err);
    }
    MIDIOutTimer = 0;
  }
#endif

  //slowing it down with interval
  if (currentTime - previousTime >= interval) {
    for (int i = 0; i < numEffects; i++) {
      if (effects[i]->finished()) {
        delete effects[i];
        removeEffect(effects[i]);
      } else {
        effects[i]->nextStep();
      }
    }
    previousTime = currentTime;
  }

  if (currentTime - previousFadeTime >= fadeInterval) {
    if (numEffects > 0 || generalFadeRate > 0) {
      fadeCtrl->fade(generalFadeRate);
    }
    previousFadeTime = currentTime;
  }

  switch (MODE) {
    case COMMAND_ANIMATION:
      Animatons(animationIndex);
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; /* motion speed */
      FillLEDsFromPaletteColors(startIndex);
      break;


    // Add more cases for additional modes or functionality here

    default:
      // Handle unknown mode or do nothing
      break;
  }

  FastLED.show();
}


void controlLeds(int ledNo, int hueVal, int saturationVal, int brightnessVal) {
  if (ledNo < 0 || ledNo >= NUM_LEDS) {
    Serial.println("Invalid LED index");
    return;
  }

  Serial.print("Control LED: ");
  Serial.print(ledNo);
  Serial.print(" HSB: ");
  Serial.print(hueVal);
  Serial.print(", ");
  Serial.print(saturationVal);
  Serial.print(", ");
  Serial.println(brightnessVal);

  // Convert HSB values to RGB values
  CRGB color = CHSV(hueVal, saturationVal, brightnessVal);
  leds[ledNum(ledNo)] = color;  // Set the LED color
  FastLED.show();               // Update the LEDs with the new color
}

int mapMidiNoteToLED(int midiNote, int lowestMidiNote, int highestMidiNote, int endIndex) {

  // Calculate the LED index using linear mapping
  int startIndex = 0;

  // Define the threshold notes where the shifts will occur
  int shiftThreshold1 = 57; // MIDI note for A3
  int shiftThreshold2 = 93; // MIDI note for C7

  // Calculate the LED index using linear mapping
  int ledIndex = map(midiNote, lowestMidiNote, highestMidiNote, startIndex, endIndex - 1);

  // Check if the useFix is equal to 1
  if (useFix == 1) {
    // Check if the MIDI note is beyond the first threshold for shifting
    if (midiNote >= shiftThreshold1) {
      // Shift all LEDs, including the A3 LED, to the left by 1 LED
      ledIndex -= 1;
    }

    // Check if the MIDI note is beyond the second threshold for shifting
    if (midiNote >= shiftThreshold2) {
      // Shift all LEDs, including the 93 MIDI note LED, to the left by 1 LED
      ledIndex -= 1;
    }
  }

  if (pianoScaleRatio == 1) {
    return startIndex + (midiNote - lowestNote);
  }
  else
  {
    return ledIndex;
  }

}



void noteOn(uint8_t note, uint8_t velocity) {
  int ledIndex = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS); // Map MIDI note to LED index
  keysOn[ledIndex] = true;

  if (serverMode == 0) {
    controlLeds(ledIndex, hue, saturation, brightness);  // Both use the same index
  } else if (serverMode == 1) {
    CHSV hsv(hue, 255, 255);
    addEffect(new FadingRunEffect(splashMaxLength, ledIndex, hsv, SPLASH_HEAD_FADE_RATE, velocity));
  } else if (serverMode == 2) {
    hue = random(256);
    controlLeds(ledIndex, hue, saturation, brightness);  // Both use the same index
  } else if (serverMode == 3) {
    int hue, saturation, brightness;
    setColorFromVelocity(velocity, hue, saturation, brightness);
    controlLeds(ledIndex, hue, saturation, brightness);
  }
  Serial.println("Note On: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void noteOff(uint8_t note, uint8_t velocity) {
  int ledIndex = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS); // Map MIDI note to LED index
  keysOn[ledIndex] = false;
  Serial.println("Note Off: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void changeLEDColor() {
  hue = random(256);

  Serial.print("Color Changed! ");
  Serial.println(hue);
}

void sliderAction(int sliderNumber, int value) {
  if (sliderNumber == 1) {
    clientHueVal = value;
    hue = clientHueVal;
  } else if (sliderNumber == 2) {
    FastLED.setBrightness(value);
  }

  else if (sliderNumber == 3) {
    generalFadeRate = value;
  }

  else if (sliderNumber == 4) {
    splashMaxLength = value;
  }
  Serial.print("Slider ");
  Serial.print(sliderNumber);
  Serial.print(" Value: ");
  Serial.println(value);
}

//Change LED Mode
void changeLEDModeAction(int serverMode) {
  blackout();
  generalFadeRate = 255;

  //Default Mode
  if (serverMode == 0) {
    MODE = COMMAND_SET_COLOR;
    hue = clientHueVal;
    saturation = clientSaturationVal;
    brightness = clientBrightnessVal;
  }
  //Splash Mode
  else if (serverMode == 1) {
    generalFadeRate = 50;
    MODE = COMMAND_SPLASH;

  }

  //Velocity Mode
  else if (serverMode == 3) {
    MODE = COMMAND_VELOCITY;
  }
  //Animation Mode
  else if (serverMode == 4) {
    MODE = COMMAND_ANIMATION;
    generalFadeRate = 0;
    animationIndex = 0;
  }
}
void blackout() {
  fill_solid(leds, NUM_LEDS, bgColor);
  MODE = COMMAND_BLACKOUT;
}

void setColorFromVelocity(int velocity, int& hue, int& saturation, int& brightness) {
  static int previousVelocity = 0;

  // Calculate the smoothed velocity as a weighted average
  int smoothedVelocity = (velocity + previousVelocity * 3) / 4;
  previousVelocity = smoothedVelocity;

  // Map smoothed velocity to hue value (green is 0° and red is 120° in HSV color space)
  hue = map(smoothedVelocity, 16, 80, 75, 255);

  // Clamp the hue value within the valid range
  hue = constrain(hue, 75, 255);

  // Map smoothed velocity to brightness value (higher velocity means higher brightness)
  brightness = map(smoothedVelocity, 16, 80, 65, 255);

  // Clamp the brightness value within the valid range
  brightness = constrain(brightness, 65, 255);

  // Set saturation to a fixed value (e.g., 255 for fully saturated color)
  saturation = 255;
}


// Add a new effect
void addEffect(FadingRunEffect * effect) {
  if (numEffects < MAX_EFFECTS) {
    effects[numEffects] = effect;
    numEffects++;
  }
}

// Remove an effect
void removeEffect(FadingRunEffect * effect) {
  for (int i = 0; i < numEffects; i++) {
    if (effects[i] == effect) {
      // Shift the remaining effects down
      for (int j = i; j < numEffects - 1; j++) {
        effects[j] = effects[j + 1];
      }
      numEffects--;
      break;
    }
  }
}

void setBG(CRGB colorToSet) {

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = colorToSet;
  }
  bgColor = colorToSet;
  FastLED.show();
}

void setIPLeds()
{
  IPAddress localIP = WiFi.localIP();
  String ipStr = localIP.toString();

   // Define colors
  CRGB redColor = CRGB(255, 0, 0);   // Red
  CRGB blueColor = CRGB(0, 0, 255);  // Blue
  CRGB blackColor = CRGB(0, 0, 0);   // Black (off)
  CRGB whiteColor = CRGB(255, 255, 255); // White

  // Define LED index and spacing
  int ledIndex = 0;
  int spacing = 1;

  // Loop through each character in the IP address
  for (int i = 0; i < ipStr.length(); i++) {
    char c = ipStr.charAt(i);

    if (c == '.') {
      // Display a blue LED for the dot
      leds[ledIndex] = blueColor;
      ledIndex++;
    } else if (c == '0') {
      // Display white LED for 0
      leds[ledIndex] = whiteColor;
      ledIndex++;
    } else if (c >= '1' && c <= '9') {
      // Convert character to an integer
      int number = c - '0';

      // Display red LEDs for other numbers
      for (int j = 0; j < number; j++) {
        leds[ledIndex] = redColor;
        ledIndex++;
      }
    }

    // Display black LED for spacing
    leds[ledIndex] = blackColor;
    ledIndex++;
  }

  // Show the entire IP address
  generalFadeRate = 0;
  FastLED.show();
}
