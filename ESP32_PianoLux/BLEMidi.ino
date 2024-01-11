#include <Arduino.h>
#if CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32 || CURRENT_BOARD_TYPE == BOARD_TYPE_ESP32S3
// Function to check if the BLE connection is stable
bool isBLEConnectionStable()
{
  static unsigned long connectionStartTime = millis();
  return (BLEMidiClient.isConnected() && (millis() - connectionStartTime > 30000));
}

void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOn(note, velocity);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  noteOff(note, velocity);
}

void handleBLE_MIDI()
{
  // Check if BLE connection is lost or unstable
  if (!BLEMidiClient.isConnected() || !isBLEConnectionStable()) {
    Serial.println("Trying to connect to BLE MIDI Device...");

    // Try to reconnect to the first BLE Midi device found
    uint8_t nDevices = BLEMidiClient.scan();
    if (nDevices > 0) {
      if (BLEMidiClient.connect(0)) {
        Serial.println("Connected!");
      } else {
        Serial.println("Connection failed");
        delay(3000);  // Wait 3s before attempting a new connection
      }
    }
  }
}
#endif
