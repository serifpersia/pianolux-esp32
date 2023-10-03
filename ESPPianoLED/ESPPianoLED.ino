#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <usb/usb_host.h>
#include "show_desc.hpp"
#include "usbhhelp.hpp"
//#define MIDIOUTTEST 1
#if MIDIOUTTEST
#include <elapsedMillis.h>
elapsedMillis MIDIOutTimer;
#endif
#include <FastLED.h>
#include "FadingRunEffect.h"
#include "FadeController.h"

// Constants for LED strip
#define MAX_NUM_LEDS 176         // How many LEDs do you want to control
#define DATA_PIN 18              // Your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  // Define current limit if you are using 5V pin from Arduino
#define MAX_EFFECTS 128

// SSID and password of Wifi connection:
const char *ssid = "";      // Your WiFi SSID
const char *password = "";  // Your WiFi password

// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

int DEFAULT_BRIGHTNESS = 255;
int NUM_LEDS = 176;             // How many LEDs you want to control
int STRIP_DIRECTION = 0;        // 0 - left-to-right

int NOTES = 12;
int generalFadeRate = 255;        // General fade rate, bigger value means quicker fade (configurable via App)
int numEffects = 0;
int lowestNote = 21;    // MIDI note A0
int highestNote = 108;  // MIDI note C8 (adjust as needed)

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

uint8_t hue;
uint8_t brightness = 255;

uint8_t Slider1Value = 0;
uint8_t Slider2Value = 255;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long previousFadeTime = 0;

unsigned long interval = 20;      // General refresh interval in milliseconds
unsigned long fadeInterval = 20;  // General fade interval in milliseconds

const int builtInLedPin = 2; // Pin for built-in LED
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
  
  Serial.begin(115200);  // init serial port for debugging
  
  usbh_setup(show_config_desc_full); //init usb host for midi devices
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);         // GRB ordering
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);  // set power limit
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

  pinMode(builtInLedPin, OUTPUT);

  StartupAnimation();

  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());

  // Serve HTML from ESP32 SPIFFS data directory
  if (SPIFFS.begin()) {
    server.serveStatic("/", SPIFFS, "/");
    server.onNotFound([](AsyncWebServerRequest *request) {
      if (request->url() == "/") {
        request->send(SPIFFS, "/index.html", "text/html");
      } else {
        request->send(404, "text/plain", "Not Found");
      }
    });
  } else {
    Serial.println("Failed to mount SPIFFS file system");
  }

  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  
  currentTime = millis();
  
  webSocket.loop();  // Update function for the webSockets
  usbh_task();

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
  int outMax = outMin + stripLEDNumber - 1;  // Highest LED number
  return map(midiNote, lowestNote, highestNote, outMin, outMax);
}

void noteOn(int note) {
  int ledIndex = mapMidiNoteToLED(note, 21, 108, 175); // Map MIDI note to LED index
    keysOn[ledIndex] = true;
    controlLeds(ledIndex, hue, 255, brightness); // Both use the same index
    digitalWrite(builtInLedPin, HIGH); // Turn on the built-in LED
    Serial.println("Note On: " + String(note) + " mapped to LED: " + String(ledIndex)); // Debug print  
}

void noteOff(int note) {
  int ledIndex = mapMidiNoteToLED(note, 21, 108, 175); // Map MIDI note to LED index
    keysOn[ledIndex] = false;
    digitalWrite(builtInLedPin, LOW); // Turn off the built-in LED
    Serial.println("Note Off: " + String(note) + " mapped to LED: " + String(ledIndex)); // Debug print
}


void changeLEDColor() {
  hue = random(256);

  Serial.print("Color Changed! ");
  Serial.println(hue);
}

void sliderAction(int sliderNumber, int value) {
  if (sliderNumber == 1) {
    hue = value;
  } else if (sliderNumber == 2) {
    FastLED.setBrightness(value);
  }

    else if (sliderNumber == 3) {
    generalFadeRate = value;
  }
  Serial.print("Slider ");
  Serial.print(sliderNumber);
  Serial.print(" Value: ");
  Serial.println(value);
}

// Add a new effect
void addEffect(FadingRunEffect* effect) {
  if (numEffects < MAX_EFFECTS) {
    effects[numEffects] = effect;
    numEffects++;
  }
}

// Remove an effect
void removeEffect(FadingRunEffect* effect) {
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
