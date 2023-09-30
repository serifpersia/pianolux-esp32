#include <Arduino.h>

void parseMIDI(uint8_t *data, size_t length) {
  if (length != 4) {
    Serial.println("Invalid MIDI message length");
    return;
  }

  uint8_t cableNumber = data[0] & 0x0F;
  uint8_t codeIndexNumber = (data[0] & 0xF0) >> 4;
  uint8_t statusByte = data[1];
  uint8_t note = data[2];
  uint8_t velocity = data[3];

  Serial.print("Ch");
  Serial.print(cableNumber);
  Serial.print(" ");

  if (statusByte >= 0x80 && statusByte < 0x90) {
    Serial.print("Note Off ");
  } else if (statusByte >= 0x90 && statusByte < 0xA0) {
    Serial.print("Note On ");
  } else {
    Serial.print("Unknown ");
  }

  Serial.print("Note: ");
  Serial.print(note);
  Serial.print(" Velocity: ");
  Serial.println(velocity);
}


// USB MIDI Event Packet Format (always 4 bytes)
//
// Byte 0 |Byte 1 |Byte 2 |Byte 3
// -------|-------|-------|------
// CN+CIN |MIDI_0 |MIDI_1 |MIDI_2
//
// CN = Cable Number (0x0..0xf) specifies virtual MIDI jack/cable
// CIN = Code Index Number (0x0..0xf) classifies the 3 MIDI bytes.
// See Table 4-1 in the MIDI 1.0 spec at usb.org.
//

static void midi_transfer_cb(usb_transfer_t *transfer) {
  ESP_LOGI("", "midi_transfer_cb context: %d", transfer->context);
  if (Device_Handle == transfer->device_handle) {
    int in_xfer = transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
    if ((transfer->status == 0) && in_xfer) {
      uint8_t *const p = transfer->data_buffer;
      for (int i = 0; i < transfer->actual_num_bytes; i += 4) {
        if ((p[i] + p[i + 1] + p[i + 2] + p[i + 3]) == 0) break;
        ESP_LOGI("", "midi: %02x %02x %02x %02x",
                 p[i], p[i + 1], p[i + 2], p[i + 3]);

        // Parse MIDI data
        uint8_t cableNumber = p[i] & 0x0F;
        uint8_t statusByte = p[i + 1];
        uint8_t note = p[i + 2];
        uint8_t velocity = p[i + 3];

        // Format the parsed MIDI data as a string
        char midiString[50];  // Adjust the size as needed
        snprintf(midiString, sizeof(midiString), "Ch%d %s Note: %d Velocity: %d",
                 cableNumber, (statusByte >= 0x80 && statusByte < 0x90) ? "Note Off" : "Note On",
                 note, velocity);

        // Send the parsed MIDI data to all connected clients
        webSocket.broadcastTXT(midiString);

        // Execute noteOn or noteOff based on the MIDI statusByte
        if (statusByte >= 0x80 && statusByte < 0x90) {
          noteOff(note);
        } else if (statusByte >= 0x90 && statusByte < 0xA0) {
          noteOn(note);
        }
      }
      esp_err_t err = usb_host_transfer_submit(transfer);
      if (err != ESP_OK) {
        ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
      }
    } else {
      ESP_LOGI("", "transfer->status %d", transfer->status);
    }
  }
}
