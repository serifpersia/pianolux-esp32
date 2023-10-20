void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
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
      if (action == "ChangeColor") {
        changeLEDColor();
      } else if (action == "Hue") {
        int value = doc["value"];
        sliderAction(1, value);
      } else if (action == "Brightness") {
        int value = doc["value"];
        sliderAction(2, value);
      } else if (action == "Fade") {
        int value = doc["value"];
        sliderAction(3, value);
      }
      else if (action == "Splash") {
        int value = doc["value"];
        sliderAction(4, value);
      } else if (action == "ChangeLEDModeAction") {
        serverMode = doc["mode"];
        changeLEDModeAction(serverMode);
        Serial.println("LED ID: ");
        Serial.print(serverMode);
      }
      else if (action == "ChangeAnimationAction")
      {
        animationIndex = doc["animation"];
        Serial.println("Animation ID: ");
        Serial.print(animationIndex);
      }
      else if (action == "Background")
      {
        int value = doc["value"];
        bgBrightness = value;
      }

      else if (action == "FixAction")
      {
        int value = doc["value"];
        if (value == 1)
        {
          useFix = 1;
        }
        else if (value == 0)
        {
          useFix = 0;
        }
      }

      else if (action == "LedScaleRatioAction")
      {
        int value = doc["value"];
        pianoScaleRatio = value;
      }

      else if (action == "PianoSizeAction")
      {
        int value = doc["value"];

        switch (value) {
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
      }
      else if (action == "DirectionAction")
      {
        int value = doc["value"];
        if (value == 1)
        {
          STRIP_DIRECTION = 1;
        }
        else if (value == 0)
        {
          STRIP_DIRECTION = 0;
        }
      }
      else if (action == "BGAction")
      {
        int value = doc["value"];
        if (value == 1)
        {
          setBG(CHSV(hue, saturation, bgBrightness));
        }
        else if (value == 0)
        {
          setBG(CHSV(0, 0, 0));
        }
      }
      break;
  }
}
