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
      StaticJsonDocument<200> doc;
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
        ledCurrent = doc["value"];
        FastLED.setMaxPowerInVoltsAndMilliamps(5, ledCurrent);
      } else if (action == "LedDataPinAction") {
        ledPin = doc["value"];
        updateGPIOConfig(ledPin);
        delay(3000);    // Debounce the button
        ESP.restart();  // Restart the ESP32
      } else if (action == "PianoSizeAction") {
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
      } else if (action == "RequestValues") {
        sendValues();
      }
      break;
  }
}

void sendValues() {
  // Create a JSON document to hold the current state
  StaticJsonDocument<400> doc;

  doc["MODES"] = serverMode;
  doc["ANIMATIONS"] = animationIndex;
  doc["COLORS"] = colorIndex;
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
  doc["CURRENT"] = ledCurrent;
  doc["LEDPIN"] = ledPin;

  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // Send the JSON data to all connected clients
  Serial.println("Sending Data To Clients");
  webSocket.broadcastTXT(jsonStr);
}
