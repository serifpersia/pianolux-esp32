#include <ArduinoJson.h>

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
      } else if (action == "SliderAction1") {
        int value = doc["value"];
        sliderAction(1, value);
      } else if (action == "SliderAction2") {
        int value = doc["value"];
        sliderAction(2, value);
      } else if (action == "SliderAction3") {
        int value = doc["value"];
        sliderAction(3, value);
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
      break;
  }
}
