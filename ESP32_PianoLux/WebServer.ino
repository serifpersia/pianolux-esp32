bool inUse = false;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {

    case WStype_CONNECTED:
      numConnectedClients++;
      if (numConnectedClients == 1 && !inUse) {
        changeLEDModeAction(0);
        inUse = true;

        // Send file list and current playback status to the newly connected client
        String fileList = listFilesJSON();
        webSocket.sendTXT(num, fileList);
        sendPlaybackStatus(num);
      }
      break;

    case WStype_DISCONNECTED:
      numConnectedClients--;
      break;

    case WStype_TEXT: {
        // WebSerial.printf("[%u] Received Text: %s\n", num, payload); // Optional log
        DynamicJsonDocument doc(1024); // Adequately sized JSON buffer - Increased size for larger messages
        DeserializationError error = deserializeJson(doc, payload, length); // Use length

        if (error) {
          //WebSerial.printf("[%u] JSON Deserialization failed: %s\n", num, error.c_str());
          // Optionally send an error back to the client
          String errorMsg = "{\"status\":\"error\", \"message\":\"Invalid JSON command received.\"}";
          webSocket.sendTXT(num, errorMsg);
          return; // Stop processing this message
        }

        // Check for command field.  If it exists, it means its the second webSocketEvent function's logic
        if (doc.containsKey("command")) {
          const char* command = doc["command"];
          if (!command) {
            //WebSerial.printf("[%u] Received JSON without 'command' field.\n", num);
            return; // Ignore if no command field
          }

          // --- Process Commands ---
          if (strcmp(command, "playMidi") == 0) {
            const char* filename = doc["filename"];
            if (!filename) {
              WebSerial.printf("[%u] 'playMidi' command missing 'filename'.\n", num);
              notifyClients("{\"status\":\"error\", \"message\":\"Play command missing filename.\"}");
              return;
            }

            //WebSerial.printf("[%u] Play command received for: %s\n", num, filename);
            String path = "/" + String(filename);

            if (!LittleFS.exists(path)) {
              //WebSerial.printf("[%u] File not found: %s\n", num, path.c_str());
              notifyClients("{\"status\":\"error\", \"message\":\"MIDI file not found: " + String(filename) + "\"}");
              return; // Don't proceed
            }

            // loadFile stops the player first if necessary
            if (midiPlayer.loadFile(path.c_str())) {
              currentLoadedFile = filename; // Store the name of the successfully loaded file
              midiPlayer.play();
              //WebSerial.printf("Playing MIDI: %s\n", filename);
              notifyClients("{\"status\":\"info\", \"message\":\"Playing MIDI: " + String(filename) + "\"}");
              sendPlaybackStatus(); // Broadcast the new PLAYING state
            } else {
              //WebSerial.printf("Failed to load MIDI: %s\n", filename);
              currentLoadedFile = ""; // Clear filename on load failure
              notifyClients("{\"status\":\"error\", \"message\":\"Failed to load/parse MIDI: " + String(filename) + "\"}");
              // loadFile calls stop() internally on failure, so state is already STOPPED
              sendPlaybackStatus(); // Broadcast the STOPPED state
            }

          } else if (strcmp(command, "stopMidi") == 0) {
            WebSerial.printf("[%u] Stop command received.\n", num);
            midiPlayer.stop();
            // currentLoadedFile = ""; // Decide if you want to clear the filename on stop
            notifyClients("{\"status\":\"info\", \"message\":\"MIDI playback stopped.\"}");
            sendPlaybackStatus(); // Broadcast the STOPPED state

          } else if (strcmp(command, "pauseMidi") == 0) {
            WebSerial.printf("[%u] Pause command received.\n", num);
            midiPlayer.pause();
            // Check if pause was successful (player must have been playing)
            if (midiPlayer.getState() == MidiPlayerState::PAUSED) {
              notifyClients("{\"status\":\"info\", \"message\":\"MIDI playback paused.\"}");
              sendPlaybackStatus(); // Broadcast the PAUSED state
            } else {
              //WebSerial.printf("[%u] Pause ignored (was not playing).\n", num);
              // Optionally send a status back if pause was ignored
            }

          } else if (strcmp(command, "resumeMidi") == 0) {
            WebSerial.printf("[%u] Resume command received.\n", num);
            midiPlayer.resume();
            // Check if resume was successful (player must have been paused)
            if (midiPlayer.getState() == MidiPlayerState::PLAYING) {
              notifyClients("{\"status\":\"info\", \"message\":\"MIDI playback resumed.\"}");
              sendPlaybackStatus(); // Broadcast the PLAYING state
            } else {
              //WebSerial.printf("[%u] Resume ignored (was not paused).\n", num);
              // Optionally send a status back if resume was ignored
            }
          } else if (strcmp(command, "deleteFile") == 0) {
            const char* filename = doc["filename"];
            deleteFileViaWebSocket(num, filename); // Call custom function
          }
          else if (strcmp(command, "getStatus") == 0) {
            //WebSerial.printf("[%u] GetStatus command received.\n", num);
            // Send current status only to the requesting client
            sendPlaybackStatus(num);
          } else {
            //WebSerial.printf("[%u] Unknown command received: %s\n", num, command);
            // Optionally notify client about unknown command
            // String errorMsg = "{\"status\":\"error\", \"message\":\"Unknown command: " + String(command) + "\"}";
            // webSocket.sendTXT(num, errorMsg);
          }
        } else { //First webSocketEvent Function

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
        }
        break; // End WStype_TEXT case
      }
      // Handle other WStype cases like WStype_BIN, WStype_ERROR, WStype_FRAGMENT_*, etc. if needed
  } // End switch (type)
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

void notifyClients(const String& message) {
  // WebSerial.println("Broadcasting WS message: " + message); // Optional log
  String localMessage = message; // Create a modifiable local copy
  webSocket.broadcastTXT(localMessage);
}

String listFilesJSON() {
  String json = "[";
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    //WebSerial.println("ERROR: Failed to open root directory!");
    return "[]";
  }
  File file = root.openNextFile();
  bool firstFile = true;
  while (file) {
    if (!file.isDirectory()) {
      if (!firstFile) json += ",";
      String filename = file.name();
      if (filename.startsWith("/")) filename = filename.substring(1);
      json += "{\"name\":\"" + filename + "\",\"size\":" + String(file.size()) + "}";
      firstFile = false;
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  json += "]";
  return json;
}


void handleFileList(AsyncWebServerRequest *request) {
  WebSerial.println("API: Request for file list");
  request->send(200, "application/json", listFilesJSON());
}

File uploadFile;
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String path = "/" + filename;
  if (filename.indexOf('/') != -1 || filename.indexOf('\\') != -1 || filename == "" || filename == "..") {
    //WebSerial.printf("UPLOAD: Invalid filename: %s\n", filename.c_str());
    if (final && uploadFile) {
      uploadFile.close();
      LittleFS.remove(path);
    }
    return;
  }
  if (index == 0) {
    //WebSerial.printf("UPLOAD START: %s\n", filename.c_str());
    if (uploadFile) uploadFile.close();
    uploadFile = LittleFS.open(path, "w");
    if (!uploadFile) {
      //WebSerial.printf("UPLOAD ERROR: Could not open %s\n", path.c_str());
      notifyClients("{\"status\":\"error\", \"message\":\"Failed to open file for upload\"}");
      return;
    }
  }
  if (uploadFile) {
    size_t bytesWritten = uploadFile.write(data, len);
    if (bytesWritten != len) {
      //WebSerial.println("UPLOAD ERROR: File write failed!");
      uploadFile.close();
      notifyClients("{\"status\":\"error\", \"message\":\"File write error during upload\"}");
      return;
    }
  }
  if (final) {
    WebSerial.printf("UPLOAD END: %s, Total Size: %u\n", filename.c_str(), index + len);
    if (uploadFile) {
      uploadFile.close();
      notifyClients(listFilesJSON());
    }
  }
}

bool handleFileRead(AsyncWebServerRequest *request) {
  String path = request->url();
  WebSerial.println("FS Request: " + path);
  if (path.endsWith("/")) path += "index.html";
  if (path.indexOf("..") != -1) return false;
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz)) {
    const AsyncWebHeader* acceptEncoding = request->getHeader("Accept-Encoding");
    if (acceptEncoding != nullptr && acceptEncoding->value().indexOf("gzip") != -1) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, pathWithGz, contentType);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return true;
    }
  }
  if (LittleFS.exists(path)) {
    request->send(LittleFS, path, contentType);
    return true;
  }
  return false;
}

String getContentType(String filename) {
  if (filename.endsWith(".mid")) return "audio/midi";
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else if (filename.endsWith(".zip")) return "application/zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

void deleteFileViaWebSocket(uint8_t clientNum, const char* filename) {
  if (!filename || strlen(filename) == 0) {
    WebSerial.printf("[%u] 'deleteFile' command missing 'filename'.\n", clientNum);
    notifyClients("{\"status\":\"error\", \"message\":\"Delete command missing filename.\"}");
    return;
  }

  WebSerial.printf("[%u] Delete command received for: %s\n", clientNum, filename);
  String path = "/" + String(filename);

  // Validate filename
  if (String(filename).indexOf('/') != -1 || String(filename).indexOf('\\') != -1 || filename == "" || filename == "..") {
    notifyClients("{\"status\":\"error\", \"message\":\"Invalid filename: " + String(filename) + "\"}");
    return;
  }

  // Check if file exists
  if (!LittleFS.exists(path)) {
    notifyClients("{\"status\":\"error\", \"message\":\"File not found: " + String(filename) + "\"}");
    return;
  }

  // Attempt to delete the file
  if (LittleFS.remove(path)) {
    WebSerial.println("API: File deleted: " + path);
    notifyClients("{\"status\":\"ok\", \"message\":\"File deleted: " + String(filename) + "\"}");
    notifyClients(listFilesJSON()); // Broadcast updated file list
  } else {
    notifyClients("{\"status\":\"error\", \"message\":\"Failed to delete file: " + String(filename) + "\"}");
  }
}
