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

//WIFI Libs
#include <WiFiManager.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

//ESP Storage Lib
#include <SPIFFS.h>

//FASTLED
#include <FastLED.h>

// Constants for LED strip
#define UPDATES_PER_SECOND 60
#define MAX_NUM_LEDS 176         // How many LEDs do you want to control
#define DATA_PIN 18              // Your LED strip data pin
#define MAX_POWER_MILLIAMPS 450  // Define current limit
int NUM_LEDS = 176;       // How many LEDs you want to control
CRGB leds[MAX_NUM_LEDS];


// Initialization of webserver and websocket
AsyncWebServer server(80);

const char* homePage =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<style>"
  "body {"
  "  font-family: Arial, sans-serif;"
  "  text-align: center;"
  "  background-color: #f0f0f0;"
  "}"
  "form {"
  "  margin-top: 20px;"
  "}"
  "input[type='submit'] {"
  "  background-color: #007bff;"
  "  color: #fff;"
  "  padding: 10px 20px;"
  "  font-size: 20px;"
  "  border: none;"
  "}"
  "input[type='submit']:hover {"
  "  background-color: #0056b3;"
  "}"
  "</style>"
  "<title>PianoLED_b</title>"
  "</head>"
  "<body>"
  "<form action='/update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "</body>"
  "</html>";


void setup() {

  Serial.begin(115200);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);         // GRB ordering
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);  // set power limit

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

  // Serve HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", homePage);
  });

  AsyncElegantOTA.begin(&server);

  server.begin();

  // Add service to mDNS for HTTP
  MDNS.addService("http", "tcp", 80);

  setIPLeds();
}

void loop() {

  AsyncElegantOTA.loop();
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
  FastLED.show();
}
