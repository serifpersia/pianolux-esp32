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
      else if (action == "PianoSizeAction")
      {
        int value = doc["value"];
        pianoSizeIndex = value;
      }
      else if (action == "LedScaleRatioAction")
      {
        int value = doc["value"];
        pianoScaleRatio = value;
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
