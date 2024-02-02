/*
  PianoLux on ESP32S2/S3 boards is an open-source project that aims to provide MIDI-based LED control to the masses.
  It is developed by a one-person team, yours truly, known as serifpersia, or Scarlett.

  If you modify this code and redistribute the PianoLux project, please ensure that you
  don't remove this disclaimer or appropriately credit the original author of the project
  by linking to the project's source on GitHub: github.com/serifpersia/pianolux-esp32/
  Failure to comply with these terms would constitute a violation of the project's
  MIT license under which PianoLux is released.

  Copyright © 2023 Serif Rami, also known as serifpersia.

*/

// PianoLux

//Chose correct board under board manager
//If USB MIDI doesn't work replace s2 or s3 esp32 sdk with modified sdk
//usb max transfer descriptor value changed from default 256 to 4096 byte
//https://drive.google.com/drive/folders/1WlxvhdeabNDGIs6hM0zICGuerMvHKQSR?usp=sharing


//Change board type that matches your ESP32
//Change Partition Scheme under Tools to Minimal SPIFFS with OTA
//before compile & upload
//Change USB mode under Tools for S3 to USB-OTG
//Default esp32 parameters can be found in config.cfg in data folder
//Change the values & save before uploading sketch data
//Upload sketch then sketch data use sketch data upload plugin
//works only with 1.8.x versions of Arduino IDE
//https://github.com/me-no-dev/arduino-esp32fs-plugin


// Define the BOARD_TYPE variable
#define BOARD_TYPE_ESP32    1
#define BOARD_TYPE_ESP32S2  2
#define BOARD_TYPE_ESP32S3  3

// Define the actual board type (change this based on your board)
#define CURRENT_BOARD_TYPE  3

//DEV defines
#define ARDUINO_OTA_YES 0
#define ARDUINO_OTA_NO 1

#define CURRENT_ARDUINO_OTA 1

// WIFI Libs
#include <WiFiManager.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

// BLE-MIDI Lib
#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
#include <BLEMidi.h>
#endif

#if CURRENT_ARDUINO_OTA == ARDUINO_OTA_YES
#include <ArduinoOTA.h>
#endif

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

// ESP Storage Lib
#include <SPIFFS.h>

// USB Lib
#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S2 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
#include <usb/usb_host.h>
#include "usbhhelp.hpp"
#endif

// rtpMIDI
#define NO_SESSION_NAME
#include <AppleMIDI.h>


//FastLED Lib
#include <FastLED.h>
#include "FadingRunEffect.h"
#include "FadeController.h"

// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

// Constants for LED strip
#define UPDATES_PER_SECOND 60
#define MAX_NUM_LEDS 176         // How many LEDs do you want to control
#define MAX_EFFECTS 128

//RMT LED STRIP INIT
#include "w2812-rmt.hpp"  // Include the custom ESP32RMT_WS2812B class
ESP32RMT_WS2812B<GRB>* wsstripGRB;
ESP32RMT_WS2812B<RGB>* wsstripRGB;
ESP32RMT_WS2812B<BRG>* wsstripBRG;

FadingRunEffect* effects[MAX_EFFECTS];
FadeController* fadeCtrl = new FadeController();


uint8_t DEFAULT_BRIGHTNESS = 255;
uint8_t NUM_LEDS = 176;       // How many LEDs you want to control
uint8_t STRIP_DIRECTION = 0;  // 0 - left-to-right

uint8_t generalFadeRate = 255;
uint8_t numEffects = 0;

uint8_t lowestNote = 21;    // MIDI note A0
uint8_t highestNote = 108;  // MIDI note C8 (adjust as needed)
uint8_t useFix;
uint8_t pianoScaleRatio;

uint8_t  getHueForPos(uint8_t pos) {
  return pos * 255 / NUM_LEDS;
}

uint8_t ledNum(uint8_t i) {
  return STRIP_DIRECTION == 0 ? i : NUM_LEDS - i;
}

CRGB leds[MAX_NUM_LEDS];
CRGB bgColor = CRGB::Black;
CRGB guideColor = CRGB::Black;

boolean bgOn = false;
boolean keysOn[MAX_NUM_LEDS];

boolean isOnStrip(uint8_t pos) {
  return pos >= 0 && pos < NUM_LEDS;
}

#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
boolean bleMIDIStarted = false;
#endif

uint8_t hue = 0;
uint8_t brightness = 255;
uint8_t saturation = 255;
uint8_t bgBrightness = 128;

#define HUE_CHANGE_SPEED 1 // Adjust this value to control the speed of hue change

uint8_t currentHue[MAX_NUM_LEDS] = {0}; // Array to store current hue value for each LED


// Define split positions (percentage)
uint8_t splitPosition = 50;  // Example: 50 means the split is in the middle
uint8_t splitLeftMinPitch = 21;
uint8_t splitRightMaxPitch = 108;

// Define split colors
CHSV splitLeftColor = CHSV(0, 255, 255);     // Red color
CHSV splitRightColor = CHSV(160, 255, 255);  // Blue color

uint8_t bgToggle;
uint8_t fixToggle;
uint8_t reverseToggle;
uint8_t bgUpdateToggle = 1;
uint8_t keySizeVal;
uint8_t colorIndex;

uint8_t LED_PIN;
uint8_t COLOR_ORDER;
uint16_t LED_CURRENT = 450;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
unsigned long previousFadeTime = 0;

unsigned long interval = 20;      // General refresh interval in milliseconds
unsigned long fadeInterval = 20;  // General fade interval in milliseconds

const uint8_t MAX_VELOCITY = 127;

const uint8_t COMMAND_SET_COLOR = 255;
const uint8_t COMMAND_FADE_RATE = 254;
const uint8_t COMMAND_ANIMATION = 253;
const uint8_t COMMAND_BLACKOUT = 252;
const uint8_t COMMAND_SPLASH = 251;
const uint8_t COMMAND_SET_BRIGHTNESS = 250;
const uint8_t COMMAND_KEY_OFF = 249;
const uint8_t COMMAND_SPLASH_MAX_LENGTH = 248;
const uint8_t COMMAND_SET_BG = 247;
const uint8_t COMMAND_VELOCITY = 246;
const uint8_t COMMAND_STRIP_DIRECTION = 245;
const uint8_t COMMAND_SET_GUIDE = 244;
const uint8_t COMMAND_SET_LED_VISUALIZER = 243;


uint8_t MODE = COMMAND_SET_COLOR;

uint8_t serverMode;

uint8_t animationIndex;

uint8_t splashMaxLength = 8;
uint8_t SPLASH_HEAD_FADE_RATE = 5;

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

float distance(CRGB color1, CRGB color2) {
  return sqrt(pow(color1.r - color2.r, 2) + pow(color1.g - color2.g, 2) + pow(color1.b - color2.b, 2));
}

void loadConfig() {
  File configFile = SPIFFS.open("/config.cfg", "r");
  if (configFile) {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, buf.get());

    // Update configuration variables
    LED_PIN = doc["LED_PIN"] | LED_PIN;
    COLOR_ORDER = doc["COLOR_ORDER"] | COLOR_ORDER;
    LED_CURRENT = doc["LED_CURRENT"] | LED_CURRENT;
    // Add more variables as needed

    configFile.close();
  } else {
    Serial.println("Failed to open config file for reading");
  }
}

void updateConfigFile(const char* configKey, uint16_t newValue) {
  DynamicJsonDocument doc(1024);

  // Read the existing config file
  File configFile = SPIFFS.open("/config.cfg", "r");
  if (configFile) {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    configFile.close();

    // Deserialize the JSON document
    deserializeJson(doc, buf.get());

    // Update the specified config key with the new value
    doc[configKey] = newValue;

    // Save the updated config file
    File updatedConfigFile = SPIFFS.open("/config.cfg", "w");
    if (updatedConfigFile) {
      serializeJson(doc, updatedConfigFile);
      updatedConfigFile.close();
      Serial.println("Config file updated successfully");
    } else {
      Serial.println("Failed to open config file for writing");
    }
  } else {
    Serial.println("Failed to open config file for reading");
  }
}

void initializeLEDStrip(uint8_t colorMode) {
  switch (colorMode) {
    case 0:
      wsstripGRB = new ESP32RMT_WS2812B<GRB>(LED_PIN);
      FastLED.addLeds(wsstripGRB, leds, NUM_LEDS);
      break;
    case 1:
      wsstripRGB = new ESP32RMT_WS2812B<RGB>(LED_PIN);
      FastLED.addLeds(wsstripRGB, leds, NUM_LEDS);
      break;
    case 2:
      wsstripBRG = new ESP32RMT_WS2812B<BRG>(LED_PIN);
      FastLED.addLeds(wsstripBRG, leds, NUM_LEDS);
      break;
    // Add more cases if needed
    default:
      // Handle default case if colorMode is not 0, 1, or 2
      break;
  }
  FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_CURRENT);  // set power limit
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

uint8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// Task handles
TaskHandle_t midiTaskHandle = NULL;

void midiTask(void* pvParameters) {
  while (1) {
    MIDI.read();                   // Handle MIDI messages
    vTaskDelay(pdMS_TO_TICKS(1));  // Adjust the delay as needed
  }
}
void StartupAnimation() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(getHueForPos(i), 255, 255);
    FastLED.show();
    leds[i] = CHSV(0, 0, 0);
  }
  FastLED.show();
}

bool startPortal = true;  // Start WiFi Manager Captive Portal

const uint8_t wmJumperPin = 15;  // Jumper pin for WiFi Manager Captive Portal
const uint8_t apJumperPin = 16;      // Jumper pin for AP mode

void startWmPortal(WiFiManager& wifiManager) {
  if (!wifiManager.startConfigPortal("PianoLux Portal")) {
    ESP.restart();
  }
}

void startAP() {
  // Start ESP32 in AP mode
  WiFi.softAP("PianoLux AP");
}

void startSTA(WiFiManager& wifiManager) {

  startPortal = false;

  // Start WiFi Manager for configuring STA mode

  //Try to connect within 5 seconds
  wifiManager.setConnectTimeout(5);
  // Set callback to be invoked when configuration is updated
  wifiManager.setSaveConfigCallback([]() {
    Serial.println("Configurations updated");
    ESP.restart();
  });

  if (!wifiManager.autoConnect("PianoLux Portal")) {
    wifiManager.resetSettings();
    ESP.restart();
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(wmJumperPin, INPUT_PULLUP);
  pinMode(apJumperPin, INPUT_PULLUP);

  // Create WiFiManager object inside setup
  WiFiManager wifiManager;

  if (digitalRead(wmJumperPin) == LOW) {
    // wmJumperPin is pulled to GND, use WiFi Manager Captive Portal
    startWmPortal(wifiManager);
  } else if (digitalRead(apJumperPin) == LOW) {
    // apJumperPin is pulled to GND, use AP mode
    startAP();
  } else {
    // None of the pins are grounded, STA mode
    startSTA(wifiManager);
  }

  //Print ESP32's IP Address
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  // Initialize and start mDNS
  if (MDNS.begin("pianolux")) {
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

#if CURRENT_ARDUINO_OTA == ARDUINO_OTA_YES

  ArduinoOTA.setHostname("PianoLux-ESP32-OTA");

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
#endif

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  // Load configuration from file
  loadConfig();

  AsyncElegantOTA.begin(&server);

  server.begin();

  // Add service to mDNS for HTTP
  MDNS.addService("http", "tcp", 80);

  // Initialize LED strip based on the loaded configuration
  initializeLEDStrip(COLOR_ORDER);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  StartupAnimation();
  if (!startPortal) {
    setIPLeds();
  }

  // Create the MIDI task
  xTaskCreatePinnedToCore(midiTask, "MIDITask", 2048, NULL, 1, &midiTaskHandle, 0);
  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const char* name) {
    isConnected++;
    Serial.print("Connected to session ");
    Serial.print(ssrc);
    Serial.print(" Name ");
    Serial.println(name);
  });

  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
    isConnected--;
    Serial.print("Disconnected ");
    Serial.println(ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    noteOn(note, velocity);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    noteOff(note, velocity);
  });

  // USB setup
#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S2 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
  usbh_setup(show_config_desc_full);  // Init USB host for MIDI devices
#endif
}

void loop() {

  // Handle USB
#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S2 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
  usbh_task();
#endif

#if CURRENT_ARDUINO_OTA == ARDUINO_OTA_YES
  ArduinoOTA.handle();
#endif

  AsyncElegantOTA.loop();
  webSocket.loop();  // Update function for the webSockets

  if (WiFi.status() == WL_CONNECTED) {
    // Call the function when WiFi is connected
    sendIP();
  }

  if (serverMode == 2) {
    // Update hue for LEDs that are currently on
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      if (keysOn[i]) {
        currentHue[i] = (currentHue[i] + HUE_CHANGE_SPEED) % 256;
        controlLeds(i, currentHue[i], saturation, brightness);
      }
    }
  }

  currentTime = millis();

  //slowing it down with interval
  if (currentTime - previousTime >= interval) {
    for (uint8_t i = 0; i < numEffects; i++) {
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
      if (animationIndex == 7) {
        // If the selected animation is 7, run the sineWave() animation
        sineWave();
      } else if (animationIndex == 8) {
        // If the selected animation is 7, run the sineWave() animation
        sparkleDots();
      } else if (animationIndex == 9) {
        // If the selected animation is 7, run the sineWave() animation
        Snake();
      } else {
        // For other animations (0 to 6), use the palette-based approach
        Animatons(animationIndex);
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */
        FillLEDsFromPaletteColors(startIndex);
      }
      break;
  }
  FastLED.show();
}

void controlLeds(uint8_t ledNo, uint8_t hueVal, uint8_t saturationVal, uint8_t brightnessVal) {
  if (ledNo < 0 || ledNo >= NUM_LEDS) {
    Serial.println("Invalid LED index");
    return;
  }
  // Convert HSB values to RGB values
  CRGB color = CHSV(hueVal, saturationVal, brightnessVal);
  leds[ledNum(ledNo)] = color;  // Set the LED color
  FastLED.show();               // Update the LEDs with the new color
}

uint8_t mapMidiNoteToLED(uint8_t midiNote, uint8_t lowestMidiNote, uint8_t highestMidiNote, uint8_t endIndex) {

  // Calculate the LED index using linear mapping
  uint8_t startIndex = 0;

  // Define the threshold notes where the shifts will occur
  uint8_t shiftThreshold1 = 57;  // MIDI note for A3
  uint8_t shiftThreshold2 = 93;  // MIDI note for C7

  // Calculate the LED index using linear mapping
  uint8_t ledIndex = map(midiNote, lowestMidiNote, highestMidiNote, startIndex, endIndex - 1);

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
  } else {
    return ledIndex;
  }
}

unsigned long previousNoteOnTime = 0;
uint8_t previousRandomHue = 0;
uint8_t firstNoteHue = 0;

void noteOn(uint8_t note, uint8_t velocity) {
  uint8_t ledIndex = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS);  // Map MIDI note to LED index
  keysOn[ledIndex] = true;

  if (serverMode == 0) {
    controlLeds(ledIndex, hue, saturation, brightness);  // Both use the same index
  } else if (serverMode == 1) {
    CHSV hsv(hue, saturation, brightness);
    addEffect(new FadingRunEffect(splashMaxLength, ledIndex, hsv, SPLASH_HEAD_FADE_RATE, velocity));
  } else   if (serverMode == 2) {
    // Check time difference between the current note-on and the previous one
    unsigned long currentTime = millis();
    unsigned long timeDifference = currentTime - previousNoteOnTime;

    // Assign the same hue as the first note within the chord time window
    if (timeDifference <= 600) {  // Adjust this chord threshold as needed
      currentHue[ledIndex] = firstNoteHue;
    } else if (timeDifference <= 50) {  // Adjust this second threshold as needed
      // Use a slightly different hue for notes close to the first note within the chord time window
      currentHue[ledIndex] = firstNoteHue + 10; // Adjust the increment value as needed
    } else {
      // Generate a new random hue for this note-on event
      uint8_t newRandomHue = random(256);
      currentHue[ledIndex] = newRandomHue;

      // Update the time and hue of the first note within the chord time window
      previousNoteOnTime = currentTime;
      firstNoteHue = newRandomHue;
    }
    controlLeds(ledIndex, currentHue[ledIndex], saturation, brightness);
  } else if (serverMode == 3) {
    uint8_t hue, saturation, brightness;
    setColorFromVelocity(velocity, hue, saturation, brightness);
    controlLeds(ledIndex, hue, saturation, brightness);
  }
  //Split Mode
  else if (serverMode == 5) {
    // Split Mode
    uint8_t splitIndex = map(note, splitLeftMinPitch, splitRightMaxPitch, 0, 100);

    if (splitIndex <= splitPosition) {
      // Use left color
      controlLeds(ledIndex, splitLeftColor.h, splitLeftColor.s, splitLeftColor.v);
    } else {
      // Use right color
      controlLeds(ledIndex, splitRightColor.h, splitRightColor.s, splitRightColor.v);
    }
  }
  Serial.println("Note On: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void noteOff(uint8_t note, uint8_t velocity) {
  uint8_t ledIndex = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS);  // Map MIDI note to LED index
  keysOn[ledIndex] = false;
  Serial.println("Note Off: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void sliderAction(uint8_t sliderNumber, uint8_t value) {
  if (sliderNumber == 1) {
    hue = value;
  } else if (sliderNumber == 2) {
    DEFAULT_BRIGHTNESS = value;
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
  } else if (sliderNumber == 3) {
    generalFadeRate = value;
  } else if (sliderNumber == 4) {
    splashMaxLength = value;
  } else if (sliderNumber == 5) {
    bgBrightness = value;
  } else if (sliderNumber == 6) {
    saturation = value;
  }
  Serial.print("Slider ");
  Serial.print(sliderNumber);
  Serial.print(" Value: ");
  Serial.println(value);
}

//Change LED Mode
void changeLEDModeAction(uint8_t serverMode) {
  blackout();
  generalFadeRate = 255;

  //Default Mode
  if (serverMode == 0) {
    MODE = COMMAND_SET_COLOR;
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
  }
}
void blackout() {
  fill_solid(leds, NUM_LEDS, bgColor);
  MODE = COMMAND_BLACKOUT;
}

void setColorFromVelocity(uint8_t velocity, uint8_t& hue, uint8_t& saturation, uint8_t& brightness) {
  static uint8_t previousVelocity = 0;

  // Calculate the smoothed velocity as a weighted average
  uint8_t smoothedVelocity = (velocity + previousVelocity * 3) / 4;
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
void addEffect(FadingRunEffect* effect) {
  if (numEffects < MAX_EFFECTS) {
    effects[numEffects] = effect;
    numEffects++;
  }
}

// Remove an effect
void removeEffect(FadingRunEffect* effect) {
  for (uint8_t i = 0; i < numEffects; i++) {
    if (effects[i] == effect) {
      // Shift the remaining effects down
      for (uint8_t j = i; j < numEffects - 1; j++) {
        effects[j] = effects[j + 1];
      }
      numEffects--;
      break;
    }
  }
}

void setBG(CRGB colorToSet) {
  fill_solid(leds, NUM_LEDS, colorToSet);
  bgColor = colorToSet;
  FastLED.show();
}

void setIPLeds() {
  IPAddress localIP = WiFi.localIP();
  String ipStr = localIP.toString();

  // Define colors
  CRGB redColor = CRGB(255, 0, 0);        // Red
  CRGB blueColor = CRGB(0, 0, 255);       // Blue
  CRGB blackColor = CRGB(0, 0, 0);        // Black (off)
  CRGB whiteColor = CRGB(255, 255, 255);  // White

  // Define LED index and spacing
  uint8_t ledIndex = 0;
  uint8_t spacing = 1;

  // Loop through each character in the IP address
  for (uint8_t i = 0; i < ipStr.length(); i++) {
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
      uint8_t number = c - '0';

      // Display red LEDs for other numbers
      for (uint8_t j = 0; j < number; j++) {
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

void sendIP() {
  if (Serial.available() > 0) {
    uint8_t command = Serial.read();
    if (command == 255) {
      // Send the local IP address back to the client
      IPAddress localIP = WiFi.localIP();
      Serial.println(localIP);
    }
  }
}
