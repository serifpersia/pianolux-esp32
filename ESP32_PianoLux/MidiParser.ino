#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
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
        uint8_t channel = p[i + 2];  // For Note On/Off, this is the note number. For CC, it's the controller number.
        uint8_t value = p[i + 3];    // For Note On/Off, this is the velocity. For CC, it's the controller value.

        // Format the parsed MIDI data as a string
        char midiString[50];  // Adjust the size as needed
        snprintf(midiString, sizeof(midiString), "Ch%d %s Channel: %d Value: %d",
                 cableNumber, (statusByte >= 0x80 && statusByte < 0x90) ? "Note Off" : (statusByte >= 0x90 && statusByte < 0xA0) ? "Note On"
                 : (statusByte >= 0xB0 && statusByte < 0xC0) ? "Control Change"
                 : "Other",
                 channel, value);

        // Process MIDI messages
        switch (statusByte & 0xF0) {
          case 0x80: // Note Off
            noteOff(channel);
            sendESP32Log("USB MIDI IN: NOTE OFF Pitch: " + String(channel) + " Velocity: " + String(value));
            if (isConnected) {
              MIDI.sendNoteOff(channel, value, 1);
              sendESP32Log("RTP MIDI Out: Note OFF " + String(channel) + " Velocity: " + String(value));
            }
            break;

          case 0x90: // Note On
            if (value == 0) {
              noteOff(channel); // Treat "Note On" with 0 velocity as "Note Off"
              sendESP32Log("USB MIDI IN: NOTE OFF Pitch: " + String(channel) + " Velocity: " + String(value));
              if (isConnected) {
                MIDI.sendNoteOff(channel, value, 1);
                sendESP32Log("RTP MIDI Out: Note OFF " + String(channel) + " Velocity: " + String(value));
              }
            } else {
              noteOn(channel, value);
              sendESP32Log("USB MIDI IN: NOTE ON Pitch: " + String(channel) + " Velocity: " + String(value));
              if (isConnected) {
                MIDI.sendNoteOn(channel, value, 1);
                sendESP32Log("RTP MIDI Out: Note ON " + String(channel) + " Velocity: " + String(value));
              }
            }
            break;

          case 0xB0: // Control Change
            // Process Control Change messages
            switch (channel) {
              case 64: // Sustain Pedal
                MIDI.sendControlChange(channel, value, 1);
                sendESP32Log("RTP MIDI Out: Sustain Pedal CC " + String(channel) + " Value: " + String(value));
                break;

              case 67: // Soft Pedal
                MIDI.sendControlChange(channel, value, 1);
                sendESP32Log("RTP MIDI Out: Soft Pedal CC " + String(channel) + " Value: " + String(value));
                break;

              case 66: // Sostenuto Pedal
                MIDI.sendControlChange(channel, value, 1);
                sendESP32Log("RTP MIDI Out: Sostenuto Pedal CC " + String(channel) + " Value: " + String(value));
                break;
            }
            break;
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
#endif
