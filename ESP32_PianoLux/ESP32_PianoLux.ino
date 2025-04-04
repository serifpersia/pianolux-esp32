/*
  PianoLux on ESP32S2/S3 boards is an open-source project that aims to provide MIDI-based LED control to the masses.
  It is developed by a one-person team, yours truly, known as serifpersia, or Scarlett.

  If you modify this code and redistribute the PianoLux project, please ensure that you
  don't remove this disclaimer or appropriately credit the original author of the project
  by linking to the project's source on GitHub: github.com/serifpersia/pianolux-esp32/
  Failure to comply with these terms would constitute a violation of the project's
  MIT license under which PianoLux is released.

  Copyright © 2023-2024 Serif Rami, also known as serifpersia.

*/

// PianoLux

// Install ESP32 arduino core sdk to be able to compile and upload code to esp32 boards
// Use my modified sdks for esp32 s2/s3 to have modified USB Descriptor value from 256 to 4096
// to support all USB MIDI devices! (replace the esp32s2 or s3 sdk folders in
// Arduino15\packages\esp32\hardware\esp32\2.0.14\tools\sdk
// link: https://drive.google.com/drive/folders/1WlxvhdeabNDGIs6hM0zICGuerMvHKQSR?usp=sharing.

// Download libs needed use the install_lib.bat script to do it
// automatically or look into that file and download zips from the links
// and extract to your Arduino/libraries folder(ESP32PartitionTool in Arduino/tools(create tools folder
// if you don't have it already).

// Restart Arduino IDE  if its open and select correct esp32 dev board.

// Change BOARD_TYPE to match your board, default board selected is ESP32S3
// Under Tools>Partition schemes select HUGE App/NO OTA/1MB SPIFFS, this lets Arduino IDE know that
// you want to use custom partitions.csv file included in the sketch folder.

// Change LED Strip Data pin from default pin 18 to some other pin your led strip is connected
// on your esp32 board, you can find this in config.cfg file in sketch location/data directory.

// Open ESP32PartitionTool, select LittleFS under SPIFFS section and press Merge binary & Upload.
// Once completed, connect your WiFi device(phone/laptop/PC with WiFi) to PianoLux Portal AP WiFi and set ESP32 to your local router network.
// Once this AP is no longer visible your ESP32 should show you your IP, you might need to recconect the board or press reset button to see it.
// Connect to PianoLux web interface via ip the link should look like http://192.168.1.32/ or use pianolux.local http://pianolux.local/.

String firmwareVersion = "v1.12";

// Define board types with unique values
#define ESP32    1
#define ESP32S2  2
#define ESP32S3  3

// Define the actual board type (change this based on your board)
#define BOARD_TYPE  ESP32S3 // select your board: ESP32, ESP32S2, or ESP32S3
#define BOARD_TYPE  ESP32S2 // select your board: ESP32, ESP32S2, or ESP32S3

#if BOARD_TYPE == ESP32S3
#include <BLEMidi.h>
#include <usb/usb_host.h>
#include "usbhhelp.hpp"
#elif BOARD_TYPE == ESP32S2
#include <usb/usb_host.h>
#include "usbhhelp.hpp"
#elif BOARD_TYPE == ESP32
#include <BLEMidi.h>
#else
#error "Unsupported board type!"
#endif

// Define flags to choose libraries
#define USE_ARDUINO_OTA 0 // Set to 1 to use ArduinoOTA, 0 to not use
#define USE_ELEGANT_OTA 1 // Set to 1 to use ElegantOTA, 0 to not use


// WIFI Libs
#include <WiFiManager.h>
#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

#if USE_ARDUINO_OTA
#include <ArduinoOTA.h>
#elif USE_ELEGANT_OTA
#include <ElegantOTA.h>
#endif

// ESP Storage Library
#include <LittleFS.h>

// rtpMIDI
#define NO_SESSION_NAME
#include <AppleMIDI.h>

#include <WebSerial.h>

//FastLED Library
#include <FastLED.h>
#include "FadingRunEffect.h"
#include "FadeController.h"


//RMT LED STRIP INIT
#include "w2812-rmt.hpp"

#include "ESP32MidiPlayer.h"

// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket(81);

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); // Creates global MIDI object

ESP32MidiPlayer midiPlayer(LittleFS); // Use LittleFS

String currentLoadedFile = ""; // Declare currentLoadedFile globally
uint8_t isConnected = 0; // Declare isConnected globally


// Constants for LED strip
#define UPDATES_PER_SECOND 60
#define MAX_NUM_LEDS 176    // How many LEDs do you want to control
#define MAX_EFFECTS 128


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
  return STRIP_DIRECTION == 0 ? i : (NUM_LEDS - 1) - i;
}

CRGB leds[MAX_NUM_LEDS];
CRGB bgColor = CRGB::Black;
CRGB guideColor = CRGB::Black;

boolean bgOn = false;
boolean keysOn[MAX_NUM_LEDS];

boolean isOnStrip(uint8_t pos) {
  return pos >= 0 && pos < NUM_LEDS;
}

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

uint8_t LED_PIN;
uint8_t COLOR_PRESET;
uint8_t COLOR_ORDER;
uint16_t LED_CURRENT = 450;

uint8_t WIFI_MODE;

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
uint8_t numConnectedClients = 0;
uint8_t splashMaxLength = 8;
uint8_t SPLASH_HEAD_FADE_RATE = 5;

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

float distance(CRGB color1, CRGB color2) {
  return sqrt(pow(color1.r - color2.r, 2) + pow(color1.g - color2.g, 2) + pow(color1.b - color2.b, 2));
}

// Function declarations
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
void sendUSBMIDINoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void sendUSBMIDINoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
void sendUSBMIDIControlChange(uint8_t channel, uint8_t controller, uint8_t value);
#endif

// Function definitions (outside midiTask())
void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  WebSerial.printf("[EVT] Note On:  Ch=%u Note=%u Vel=%u (Tick: %lu)\n",
                   channel + 1, note, velocity, midiPlayer.getCurrentTick()); // Display channel 1-16

  noteOn(note, velocity);
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
  sendUSBMIDINoteOn(channel, note, velocity);
#endif
  if (isConnected) {
    MIDI.sendNoteOn(note, velocity, channel + 1);
  }
}

void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  WebSerial.printf("[EVT] Note Off:  Ch=%u Note=%u Vel=%u (Tick: %lu)\n",
                   channel + 1, note, velocity, midiPlayer.getCurrentTick()); // Display channel 1-16

  noteOff(note);
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
  sendUSBMIDINoteOff(channel, note, velocity);
#endif
  if (isConnected) {
    MIDI.sendNoteOn(note, velocity, channel + 1);
  }
}

void handleControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  WebSerial.printf("[EVT] Control Change:  Ch=%u Controller=%u Value=%u (Tick: %lu)\n",
                   channel + 1, controller, value, midiPlayer.getCurrentTick()); // Display channel 1-16

  noteOff(note);
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
  sendUSBMIDIControlChange(channel, controller, value);
#endif
  if (isConnected) {
    MIDI.sendNoteOn(note, velocity, channel + 1);
  }
}
void handlePlaybackComplete() {
  notifyClients("{\"status\":\"info\", \"message\":\"MIDI playback finished: " + currentLoadedFile + "\"}"); // Now accessible
}

void loadConfig() {
  File configFile = LittleFS.open("/config.cfg", "r");
  if (configFile) {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf.get());

    if (error) {
      Serial.println("Failed to parse config file");
      return;
    }

    // Update configuration variables
    LED_PIN = doc["LED_PIN"] | LED_PIN;
    COLOR_ORDER = doc["COLOR_ORDER"] | COLOR_ORDER;
    LED_CURRENT = doc["LED_CURRENT"] | LED_CURRENT;
    WIFI_MODE = doc["WIFI_MODE"] | WIFI_MODE;
    // Add more variables as needed

    configFile.close();
  } else {
    Serial.println("Failed to open config file for reading");
  }
}

void updateConfigFile(const char* configKey, uint16_t newValue) {
  JsonDocument doc;

  // Read the existing config file
  File configFile = LittleFS.open("/config.cfg", "r");
  if (configFile) {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);
    configFile.close();

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, buf.get());

    if (error) {
      Serial.println("Failed to parse config file");
      return;
    }

    // Update the specified config key with the new value
    doc[configKey] = newValue;

    // Save the updated config file
    File updatedConfigFile = LittleFS.open("/config.cfg", "w");
    if (updatedConfigFile) {
      if (serializeJson(doc, updatedConfigFile) == 0) {
        Serial.println("Failed to write updated config file");
      }
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

// Function to send the current status to clients (implement this)
void sendPlaybackStatus(uint8_t num = 255) { // 255 means broadcast
  PlaybackState state = midiPlayer.getState();
  String statusStr;
  switch (state) {
    case PlaybackState::STOPPED:  statusStr = "stopped"; break;
    case PlaybackState::PLAYING:  statusStr = "playing"; break;
    case PlaybackState::PAUSED:   statusStr = "paused";  break;
    default: statusStr = "unknown"; break;
  }

  // Create JSON payload
  DynamicJsonDocument doc(256);
  doc["status"] = "playbackState"; // Use "status" as the identifier field
  doc["state"] = statusStr;
  // Only include filename if relevant (playing, paused, or just finished)
  if (state == PlaybackState::PLAYING || state == PlaybackState::PAUSED) {
    doc["filename"] = currentLoadedFile; // Use the renamed variable
  } else {
    doc["filename"] = ""; // Or null? Empty string is fine.
  }

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  if (num == 255) {
    webSocket.broadcastTXT(jsonOutput); // Send to all clients
  } else {
    webSocket.sendTXT(num, jsonOutput); // Send to specific client
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

#if USE_ARDUINO_OTA
void setupArduinoOTA()
{
  ArduinoOTA.setHostname("PianoLux-ESP32-OTA");

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_LittleFS
      type = "filesystem";

    // NOTE: if updating LittleFS this would be the place to unmount LittleFS using LittleFS.end()
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
}
#endif


void handleLog(MidiLogLevel level, const char* message) {
  const char* levelStr = "";
  switch (level) {
    case MidiLogLevel::ERROR:
      levelStr = "[ERR] "; // Errors that might halt playback or indicate corruption
      break;
    case MidiLogLevel::WARN:
      levelStr = "[WRN] "; // Warnings about unexpected data or potential issues
      break;
    case MidiLogLevel::INFO:
      levelStr = "[INF] "; // General information (playback start/stop, file loaded)
      break;
    case MidiLogLevel::DEBUG:
      levelStr = "[DBG] "; // Detailed debugging steps (event parsing, byte reads)
      break;
    case MidiLogLevel::VERBOSE:
      levelStr = "[VER] "; // Extremely detailed info (often too noisy)
      break;
    // case MidiLogLevel::NONE: // No need to handle NONE, the library checks this
    default:
      levelStr = "[???] "; // Unknown level? Should not happen.
      break;
  }
  // Print the prefix and the message, followed by a newline
  WebSerial.printf("%s%s\n", levelStr, message);
}

void setup() {

  pinMode(wmJumperPin, INPUT_PULLUP);
  pinMode(apJumperPin, INPUT_PULLUP);

  WiFiManager wifiManager;

  if (!LittleFS.begin()) {
    return;
  }

  loadConfig();

  if (digitalRead(wmJumperPin) == LOW || WIFI_MODE == 1) {
    if (WIFI_MODE == 1)
    {
      WIFI_MODE = 0;
      updateConfigFile("WIFI_MODE", WIFI_MODE);
    }
    startWmPortal(wifiManager);
  } else if (digitalRead(apJumperPin) == LOW || WIFI_MODE == 2) {
    startAP();
  } else {
    startSTA(wifiManager);
  }

  WebSerial.begin(&server);
  WebSerial.println("Booting...");


  WebSerial.println("IP address: ");
  WebSerial.println(WiFi.localIP());


  // Initialize and start mDNS
  if (MDNS.begin("pianolux")) {
    WebSerial.println("MDNS Responder Started!");
  }

  server.on("/api/storage", HTTP_GET, handleStorageInfo);
  server.on("/api/files", HTTP_GET, handleFileList);
  server.on("/api/upload", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Upload Received");
  }, handleUpload);

  server.onNotFound([](AsyncWebServerRequest * request) {
    if (!handleFileRead(request)) {
      if (request->url() == "/" && LittleFS.exists("/index.html")) {
        request->send(LittleFS, "/index.html", "text/html");
      } else {
        WebSerial.printf("Not Found: %s\n", request->url().c_str());
        request->send(404, "text/plain", "Not Found");
      }
    }
  });

#if USE_ARDUINO_OTA
  setupArduinoOTA();
#endif

#if USE_ELEGANT_OTA
  ElegantOTA.begin(&server);
#endif

  server.begin();

  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  // Initialize LED strip based on the loaded configuration
  initializeLEDStrip(COLOR_ORDER);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  StartupAnimation();
  if (!startPortal) {
    setIPLeds();
  }

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
    if (isConnected) {
      sendUSBMIDINoteOn(channel, note, velocity);
    }

    noteOn(note, velocity);

    if (numConnectedClients != 0)
    {
      //sendESP32Log("RTP MIDI IN: NOTE ON: Channel: " + String(channel) + " Pitch: " + String(note) + " Velocity: " + String(velocity));
    }
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    if (isConnected) {
      sendUSBMIDINoteOff(channel, note, velocity);
    }

    noteOff(note);

    if (numConnectedClients != 0)
    {
      //sendESP32Log("RTP MIDI IN: NOTE OFF: Channel: " + String(channel) + " Pitch: " + String(note) + " Velocity: " + String(velocity));
    }
  });

#if BOARD_TYPE == ESP3S3 || BOARD_TYPE == ESP32
  BLEMidiClient.begin("PianoLux-BLE");
  //BLEMidiClient.enableDebugging();
  BLEMidiClient.setNoteOnCallback(BLE_onNoteOn);
  BLEMidiClient.setNoteOffCallback(BLE_onNoteOff);
#endif

  // USB setup
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
  usbh_setup(show_config_desc_full);  // Init USB host for MIDI devices
#endif

  //midiPlayer.setLogCallback(handleLog);
  //midiPlayer.setLogLevel(MidiLogLevel::INFO);

  midiPlayer.setNoteOnCallback(handleNoteOn);
  midiPlayer.setNoteOffCallback(handleNoteOff);
  midiPlayer.setControlChangeCallback(handleControlChange);
  midiPlayer.setPlaybackCompleteCallback(handlePlaybackComplete);
}

void loop() {

  MIDI.read();

  // Check MIDI player state changes
  static PlaybackState lastState = midiPlayer.getState();
  PlaybackState currentState = midiPlayer.getState();
  if (currentState != lastState) {
    sendPlaybackStatus();

    // reset leds
    fill_solid(leds, NUM_LEDS, bgColor);

    lastState = currentState;
  }

  // Handle USB
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
  usbh_task();
#endif


  midiPlayer.tick(); // Ensure this is called in your loop to process events


#if USE_ARDUINO_OTA
  ArduinoOTA.handle();
#endif

#if USE_ELEGANT_OTA
  ElegantOTA.loop();
#endif

  webSocket.loop();  // Update function for the webSockets

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
  //Serial.println("Note On: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
}

void noteOff(uint8_t note) {
  uint8_t ledIndex = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS);  // Map MIDI note to LED index
  keysOn[ledIndex] = false;
  //Serial.println("Note Off: " + String(note) + " mapped to LED: " + String(ledIndex));  // Debug print
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
void addEffect(FadingRunEffect * effect) {
  if (numEffects < MAX_EFFECTS) {
    effects[numEffects] = effect;
    numEffects++;
  }
}

// Remove an effect
void removeEffect(FadingRunEffect * effect) {
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
