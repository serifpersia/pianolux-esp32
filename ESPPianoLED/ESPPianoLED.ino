#include <WiFiManager.h>
#include <FastLED.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

#include <SPIFFS.h>
#include <usb/usb_host.h>
#include "usbhhelp.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>


//#define MIDIOUTTEST 1
#if MIDIOUTTEST
#include <elapsedMillis.h>
elapsedMillis MIDIOutTimer;
#endif
#include "FadingRunEffect.h"
#include "FadeController.h"

// Constants for LED strip
#define MAX_NUM_LEDS 176         // How many LEDs do you want to control
#define DATA_PIN 18              // Your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  // Define current limit if you are using 5V pin from Arduino
#define MAX_EFFECTS 128

// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

int DEFAULT_BRIGHTNESS = 255;
int NUM_LEDS = 176;       // How many LEDs you want to control
int STRIP_DIRECTION = 0;  // 0 - left-to-right

int NOTES = 12;
int generalFadeRate = 255;  // General fade rate, bigger value means quicker fade (configurable via App)
int numEffects = 0;
int lowestNote = 21;    // MIDI note A0
int highestNote = 108;  // MIDI note C8 (adjust as needed)
int pianoSizeIndex;
int pianoScaleRatio;

int getHueForPos(int pos) {
  return pos * 255 / NUM_LEDS;
}
int getNote(int key) {
  return key % NOTES;
}


int ledNum(int i) {
  return STRIP_DIRECTION == 0 ? i : (NUM_LEDS - 1) - i;
}


int getRandomHue() {
  return random(256);
}

FadingRunEffect* effects[MAX_EFFECTS];
FadeController* fadeCtrl = new FadeController();

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

const int builtInLedPin = 2;  // Pin for built-in LED
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

#define UPDATES_PER_SECOND 120


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

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);         // GRB ordering
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);  // set power limit
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

  StartupAnimation();

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

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Add service to mDNS for HTTP
  MDNS.addService("http", "tcp", 80);

  usbh_setup(show_config_desc_full);  //init usb host for midi devices

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

int mapMidiNoteToLED(int midiNote, int lowestNote, int highestNote, int stripLEDNumber) {
  int outMin = 0;                            // Start of LED strip
  int outMax = stripLEDNumber - 1;           // Highest LED number

  if (pianoScaleRatio == 0) {
    // Every other LED mapping
    return outMin + 2 * (midiNote - lowestNote);
  } else if (pianoScaleRatio == 1) {
    // 1:1 scale mapping
    return outMin + (midiNote - lowestNote);
  }
}



void noteOn(uint8_t note, uint8_t velocity) {
  int ledIndex = mapMidiNoteToLED(note, 21, 108, 175);  // Map MIDI note to LED index
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
  digitalWrite(builtInLedPin, HIGH);                                                   // Turn on the built-in LED
  Serial.println("Note On: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void noteOff(uint8_t note, uint8_t velocity) {
  int ledIndex = mapMidiNoteToLED(note, 21, 108, 175);  // Map MIDI note to LED index
  keysOn[ledIndex] = false;
  digitalWrite(builtInLedPin, LOW);                                                     // Turn off the built-in LED
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
