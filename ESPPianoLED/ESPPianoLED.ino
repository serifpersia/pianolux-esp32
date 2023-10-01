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
// SSID and password of Wifi connection:
const char *ssid = ""; //your wifi ssid
const char *password = ""; //your wifi password

// Initialization of webserver and websocket
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int builtInLedPin = 2;  // Pin for built-in LED

//PianoLED
#include <FastLED.h>
#define MAX_NUM_LEDS 176         // how many leds do you want to control
#define DATA_PIN 18              // your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  //define current limit if you are using 5V pin from Arduino dont touch this, \

int NUM_LEDS = 176;              // how many leds do you want to control
int STRIP_DIRECTION = 0;         //0 - left-to-right

uint8_t hue;
uint8_t brightness = 255;
uint8_t Slider1Value = 0;
uint8_t Slider2Value = 255;

int DEFAULT_BRIGHTNESS = 255;

boolean keysOn[MAX_NUM_LEDS];

CRGB leds[MAX_NUM_LEDS];
int NOTES = 12;

int getHueForPos(int pos) {
  return pos * 255 / NUM_LEDS;
}
int getNote(int key) {
  return key % NOTES;
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
}

int ledNum(int i) {
  return STRIP_DIRECTION == 0 ? i : (NUM_LEDS - 1) - i;
}


int getRandomHue() {
  return random(256);
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


int lowestNote = 21;    // MIDI note A0
int highestNote = 108;  // MIDI note C8 (adjust as needed)

int mapMidiNoteToLED(int midiNote, int lowestNote, int highestNote, int stripLEDNumber) {
  int outMin = 0;                            // Start of LED strip
  int outMax = outMin + stripLEDNumber - 1;  // Highest LED number
  return map(midiNote, lowestNote, highestNote, outMin, outMax);
}



void noteOn(uint8_t note) {
  int mappedLED = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS);
  keysOn[note - 21] = true;
  controlLeds(mappedLED, hue, 255, brightness);  // Pass the mapped LED index
  digitalWrite(builtInLedPin, HIGH);             // Turn on the built-in LED
  Serial.println("Note On: " + String(note));    // Debug print
}

void noteOff(uint8_t note) {
  int mappedLED = mapMidiNoteToLED(note, lowestNote, highestNote, NUM_LEDS);
  keysOn[note - 21] = false;
  controlLeds(mappedLED, 0, 0, 0);              // Pass the mapped LED index
  digitalWrite(builtInLedPin, LOW);             // Turn off the built-in LED
  Serial.println("Note Off: " + String(note));  // Debug print
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
    brightness = value;
  }

  Serial.print("Slider ");
  Serial.print(sliderNumber);
  Serial.print(" Value: ");
  Serial.println(value);
}
