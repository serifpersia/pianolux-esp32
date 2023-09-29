#include <WiFi.h>              // needed to connect to WiFi
#include <WebServer.h>         // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets

#include <usb/usb_host.h>
#include "show_desc.hpp"
#include "usbhhelp.hpp"

//#define MIDIOUTTEST 1
#if MIDIOUTTEST
#include <elapsedMillis.h>
elapsedMillis MIDIOutTimer;
#endif
// SSID and password of Wifi connection:
const char *ssid = "Wifi 2.4Ghz"; //type your router wifi name
const char *password = "cigani123"; //your password for that wifi

String website = "<!DOCTYPE html><html><head><title>Page Title</title></head><body style='background-color: #EEEEEE;'><span style='color: #003366;'><h1>MIDI Messages</h1><p>MIDI Data: <span id='rand'></span></p><button type='button' id='BTN_COLOR'>Change Color</button></span></body><script> var Socket; document.getElementById('BTN_COLOR').addEventListener('click', button_changeColor); function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function(event) { processCommand(event); }; } function button_changeColor() { Socket.send('ChangeColor'); } function processCommand(event) { var message = event.data; if (message === 'ChangeColor') { console.log('Changing LED color...'); } else { document.getElementById('rand').innerHTML = message; } } window.onload = function(event) { init(); }</script></html>";


// Initialization of webserver and websocket
WebServer server(80);                               // the server uses port 80 (standard port for websites
WebSocketsServer webSocket = WebSocketsServer(81);  // the websocket uses port 81 (standard port for websockets

const int builtInLedPin = 2;  // Pin for built-in LED

//PianoLED

#include <FastLED.h>

#define MAX_NUM_LEDS 176         // how many leds do you want to control
#define DATA_PIN 18              // your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  //define current limit if you are using 5V pin from Arduino dont touch this, \

int NUM_LEDS = 176;              // how many leds do you want to control
int STRIP_DIRECTION = 0;         //0 - left-to-right

unsigned long currentTime = 0;

const int COMMAND_BYTE1 = 111;
const int COMMAND_BYTE2 = 222;

int MODE;

const int COMMAND_SET_COLOR = 255;
const int COMMAND_KEY_OFF = 249;

int bufferSize;
int buffer[10];  // declare buffer as an array of 10 integers
int bufIdx = 0;  // initialize bufIdx to zero
int generalBrightness = buffer[++bufIdx];

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

  usbh_setup(show_config_desc_full);

  Serial.begin(115200);  // init serial port for debugging

  Serial.setTimeout(10);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);         // GRB ordering
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);  // set power limit
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

  pinMode(builtInLedPin, OUTPUT);

  StartupAnimation();

  WiFi.begin(ssid, password);                                                    // start WiFi interface
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));  // print SSID to the serial interface for debugging

  while (WiFi.status() != WL_CONNECTED) {  // wait until WiFi is connected
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());  // show IP address that the ESP32 has received from router

  server.on("/", []() {                      // define here wat the webserver needs to do
    server.send(200, "text/html", website);  //    -> it needs to send out the HTML string "webpage" to the client
  });
  server.begin();  // start server

  webSocket.begin();                  // start websocket
  webSocket.onEvent(webSocketEvent);  // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
}

void loop() {
  server.handleClient();  // Needed for the webserver to handle all clients
  webSocket.loop();       // Update function for the webSockets

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


uint8_t Hue;

void changeLEDColor() {
  // Generate a random hue value
  Hue = random(256);
}
