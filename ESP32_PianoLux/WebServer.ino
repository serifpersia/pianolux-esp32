uint8_t numConnectedClients = 0;
bool inUse = false;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {

    case WStype_CONNECTED:
      numConnectedClients++;
      if (numConnectedClients == 1 && !inUse) {
        changeLEDModeAction(0);
        inUse = true;
      }
      break;

    case WStype_DISCONNECTED:
      numConnectedClients--;
      break;

    case WStype_TEXT:
      // Parse the JSON message
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
        return;
      }

      String action = doc["action"];

      if (action == "ChangeLEDModeAction") {
        serverMode = doc["mode"];
        changeLEDModeAction(serverMode);
        Serial.println("LED ID: ");
        Serial.print(serverMode);
      } else if (action == "ChangeAnimationAction") {
        animationIndex = doc["animation"];
        Serial.println("Animation ID: ");
        Serial.print(animationIndex);
      } else if (action == "Hue") {
        uint8_t value = doc["value"];
        sliderAction(1, value);
      } else if (action == "Saturation") {
        uint8_t value = doc["value"];
        sliderAction(6, value);
      } else if (action == "Brightness") {
        uint8_t value = doc["value"];
        sliderAction(2, value);
      } else if (action == "Fade") {
        uint8_t value = doc["value"];
        sliderAction(3, value);
      } else if (action == "Splash") {
        uint8_t value = doc["value"];
        sliderAction(4, value);
      } else if (action == "Background") {
        uint8_t value = doc["value"];
        sliderAction(5, value);
      } else if (action == "CurrentAction") {
        LED_CURRENT = doc["value"];
        FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_CURRENT);
        updateConfigFile("LED_CURRENT", LED_CURRENT);
      } else if (action == "LedDataPinAction") {
        LED_PIN = doc["value"];
        updateConfigFile("LED_PIN", LED_PIN);
        delay(1000);    // Debounce the button
        ESP.restart();  // Restart the ESP32
      }
      else if (action == "ChangeColorOrderAction") {
        COLOR_ORDER = doc["colorOrder"];
        updateConfigFile("COLOR_ORDER", COLOR_ORDER);
        delay(1000);    // Debounce the button
        ESP.restart();  // Restart the ESP32
      }
      else if (action == "ColorPresetAction") {
        hue = doc["colorPresetHue"];
        saturation = doc["colorPresetSaturation"];
        COLOR_PRESET = doc["colorPresetID"];
      }
      else if (action == "PianoSizeAction") {
        keySizeVal = doc["value"];
        switch (keySizeVal) {
          case 0:
            NUM_LEDS = 176;
            lowestNote = 21;
            highestNote = 108;
            break;
          case 1:
            NUM_LEDS = 152;
            lowestNote = 28;
            highestNote = 103;
            break;
          case 2:
            NUM_LEDS = 146;
            lowestNote = 28;
            highestNote = 100;
            break;
          case 3:
            NUM_LEDS = 122;
            lowestNote = 36;
            highestNote = 96;
            break;
          case 4:
            NUM_LEDS = 98;
            lowestNote = 36;
            highestNote = 84;
            break;
        }
      } else if (action == "LedScaleRatioAction") {
        uint8_t value = doc["value"];
        pianoScaleRatio = value;
      } else if (action == "FixAction") {
        fixToggle = doc["value"];
        if (fixToggle == 1) {
          useFix = 1;
        } else if (fixToggle == 0) {
          useFix = 0;
        }
      } else if (action == "BGAction") {
        bgToggle = doc["value"];
        if (bgToggle == 1) {
          setBG(CHSV(hue, saturation, bgBrightness));
        } else if (bgToggle == 0) {
          setBG(CHSV(0, 0, 0));
        }
      } else if (action == "DirectionAction") {
        reverseToggle = doc["value"];
        if (reverseToggle == 1) {
          STRIP_DIRECTION = 1;
        } else if (reverseToggle == 0) {
          STRIP_DIRECTION = 0;
        }
      } else if (action == "BGUpdateAction") {
        bgUpdateToggle = doc["value"];
        setBG(CHSV(hue, saturation, bgBrightness));
      } else if (action == "Split") {
        splitPosition = doc["value"];
      } else if (action == "SetSplitAction") {
        uint8_t splitIndex = doc["index"];
        uint8_t splitHue = doc["hue"];
        uint8_t splitSaturation = doc["saturation"];
        uint8_t splitBrightness = doc["brightness"];
        if (splitIndex == 0) {
          // Handle left split
          splitLeftColor = CHSV(splitHue, splitSaturation, splitBrightness);
        } else if (splitIndex == 1) {
          // Handle right split
          splitRightColor = CHSV(splitHue, splitSaturation, splitBrightness);
        }
      }
      else if (action == "ScanBluetoothAction") {
#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32
        scan_BLE_MIDI();
#endif
      }
      else if (action == "ReadESP32Info") {
        sendESP32Info();
      }
      else if (action == "ReadESP32Logs") {
        sendESP32Info();
      }
      else if (action == "RequestValues") {
        sendValues();
      }
      break;
  }
}

void sendESP32Log(String logMessage) {
  JsonDocument doc;

  doc["LOG_MESSAGE"] = logMessage;

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // Send the JSON data to all connected clients
  Serial.println("Sending Data To Clients");
  webSocket.broadcastTXT(jsonStr);
}

void sendValues() {
  JsonDocument doc;

  doc["MODE"] = serverMode;
  doc["ANIMATION"] = animationIndex;
  doc["HUE"] = hue;
  doc["SATURATION"] = saturation;
  doc["BRIGHTNESS"] = DEFAULT_BRIGHTNESS;
  doc["FADE"] = generalFadeRate;
  doc["SPLASH"] = splashMaxLength;
  doc["BG"] = bgBrightness;
  doc["SPLIT"] = splitPosition;
  doc["FIX_TOGGLE"] = fixToggle;
  doc["BG_TOGGLE"] = bgToggle;
  doc["REVERSE_TOGGLE"] = reverseToggle;
  doc["BGUPDATE_TOGGLE"] = bgUpdateToggle;
  doc["LED_CURRENT"] = LED_CURRENT;
  doc["LED_PIN"] = LED_PIN;
  doc["LED_COLOR_PRESET"] = COLOR_PRESET;
  doc["LED_COLOR_ORDER"] = COLOR_ORDER;

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // Send the JSON data to all connected clients
  Serial.println("Sending Data To Clients");
  webSocket.broadcastTXT(jsonStr);
}

// Read the temperature
float readTemperature() {
  // Read the temperature from the internal sensor
  float temperatureFloat = temperatureRead();
  uint8_t  temperature = (uint8_t)temperatureFloat; // Cast to intege
  return temperature;
}

void sendESP32Info() {
  // Create a JSON document to hold the ESP32 information
  JsonDocument doc;

  doc["FirmwareVersion"] = firmwareVersion;
  doc["FirmwareBuildDate"] = __DATE__;

  // Populate device information
  doc["ChipModel"] = ESP.getChipModel();

  // Get network details
  doc["SSID"] = WiFi.SSID(); // Include SSID name in the information
  doc["IPAddress"] = WiFi.localIP().toString();
  doc["MACAddress"] = WiFi.macAddress();
  doc["Temperature"] = String(readTemperature()) + " °C";

  unsigned long totalSeconds = millis() / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  doc["Uptime"] = String(hours) + " hours, " + String(minutes) + " minutes, " + String(seconds) + " seconds";

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // Send the JSON data to all connected clients
  Serial.println("Sending ESP32 Info To Clients");
  webSocket.broadcastTXT(jsonStr);
}
