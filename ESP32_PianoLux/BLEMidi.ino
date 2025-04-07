#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32
void scan_BLE_MIDI() {
  if (!BLEMidiClient.isConnected()) {
    // If we are not already connected, we try te connect to the first BLE Midi device we find
    int nDevices = BLEMidiClient.scan();
    if (nDevices > 0) {
      if (BLEMidiClient.connect(0)) {
        WebSerial.println("Connection established");
        WebSerial.print("BLE: Connected to BLE MIDI device!");
        if (numConnectedClients != 0 && CLIENT_LOGGER) {
          sendESP32Log("BLE: Connected to BLE MIDI device!");
        }

      } else {
        WebSerial.println("BLE: Failed to connect!");
        if (numConnectedClients != 0 && CLIENT_LOGGER) {
          sendESP32Log("BLE: Failed to connect!");

        }
      }
    }
  }
}

void BLE_onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOn(note, velocity);
  if (numConnectedClients != 0 && CLIENT_LOGGER) {
    sendESP32Log("BLE MIDI IN: Channel: " + String(channel) + "" + "Note ON " + String(note) + "" + "Velocity" + String(velocity));

  }
  if (BLEMidiClient.isConnected()) {
    if (isConnected) {
      if (numConnectedClients != 0 && CLIENT_LOGGER) {
        sendESP32Log("BLE RTP MIDI Out: Note ON " + String(note) + " Velocity: " + String(velocity));
      }
      MIDI.sendNoteOn(note, velocity, 1);
    }
  }
}

void BLE_onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOff(note);

  if (numConnectedClients != 0 && CLIENT_LOGGER) {
    sendESP32Log("BLE MIDI IN: Channel: " + String(channel) + "" + "Note OFF " + String(note) + "" + "Velocity" + String(velocity));

  }

  if (BLEMidiClient.isConnected()) {
    if (isConnected) {
      if (numConnectedClients != 0 && CLIENT_LOGGER) {
        sendESP32Log("BLE RTP MIDI Out: Note OFF " + String(note) + " Velocity: " + String(velocity));
      }
      MIDI.sendNoteOff(note, velocity, 1);
    }
  }
}

void BLE_onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {

  if (numConnectedClients != 0 && CLIENT_LOGGER) {
    sendESP32Log("BLE MIDI IN: CC " + String(controller) + "" + "Value" + String(value));

  }

  if (BLEMidiClient.isConnected()) {
    if (isConnected) {
      if (numConnectedClients != 0 && CLIENT_LOGGER) {
        sendESP32Log("BLE RTP MIDI Out: CC " + String(controller) + " Value: " + String(value));
      }
      MIDI.sendControlChange(controller, value, 1);
    }
  }
}
#endif
