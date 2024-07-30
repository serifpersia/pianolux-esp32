#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32
void scan_BLE_MIDI()
{
  if (!BLEMidiClient.isConnected()) {
    // If we are not already connected, we try te connect to the first BLE Midi device we find
    int nDevices = BLEMidiClient.scan();
    if (nDevices > 0) {
      if (BLEMidiClient.connect(0)) {
        Serial.println("Connection established");
        Serial.print("BLE: Connected to BLE MIDI device!");
        sendESP32Log("BLE: Connected to BLE MIDI device!");
      } else {
        Serial.println("BLE: Failed to connect!");
        sendESP32Log("BLE: Failed to connect!");
      }
    }
  }
}

void BLE_onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOn(note, velocity);
  sendESP32Log("BLE MIDI IN: Channel: " + String(channel) + "" + "Note ON " + String(note) + "" + "Velocity" + String(velocity));
  if (BLEMidiClient.isConnected())
  {
    MIDI.sendNoteOn(note, velocity, 1);
    sendESP32Log("RTP MIDI Out: Note ON " + String(note) + " Velocity: " + String(velocity));
  }
}

void BLE_onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOff(note);
  sendESP32Log("BLE MIDI IN: Channel: " + String(channel) + "" + "Note OFF " + String(note));
  if (BLEMidiClient.isConnected())
  {
    MIDI.sendNoteOff(note, velocity, 1);
    sendESP32Log("RTP MIDI Out: Note OFF " + String(note) + " Velocity: " + String(velocity));
  }
}
#endif
