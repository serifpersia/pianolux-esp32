void handleBLE_MIDI()
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
        delay(3000);  // We wait 3s before attempting a new connection
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
