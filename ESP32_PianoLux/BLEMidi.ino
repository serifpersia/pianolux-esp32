#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3

void control_BLE_MIDIClient()
{
  if (bleMIDIStarted) {
    // Stop the MIDI client
    BLEMidiClient.end();
    bleMIDIStarted = false;
  } else {
    // Start the BLE client
    BLEMidiClient.begin("PianoLux");
    //BLEMidiClient.enableDebugging();
    BLEMidiClient.setNoteOnCallback(BLE_onNoteOn);
    BLEMidiClient.setNoteOffCallback(BLE_onNoteOff);
    bleMIDIStarted = true;
  }
}

void scan_BLE_MIDI()
{
  if (!BLEMidiClient.isConnected()) {
    // If we are not already connected, we try te connect to the first BLE Midi device we find
    int nDevices = BLEMidiClient.scan();
    if (nDevices > 0) {
      if (BLEMidiClient.connect(0)) {
        Serial.println("Connection established");
        Serial.print("Connected to MIDI device: ");
      } else {
        Serial.println("Connection failed");
      }
    }
  }
}

void BLE_onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOn(note, velocity);
}

void BLE_onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOff(note, velocity);
}
#endif
