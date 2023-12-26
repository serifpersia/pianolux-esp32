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

        // Execute noteOn or noteOff based on the MIDI statusByte
        if (statusByte >= 0x80 && statusByte < 0x90) {
          noteOff(note, velocity);
          if (isConnected) {
            MIDI.sendNoteOff(note, velocity, 1);
          }
        } else if (statusByte >= 0x90 && statusByte < 0xA0) {
          if (velocity == 0) {
            noteOff(note, velocity);  // Treat "Note On" with 0 velocity as "Note Off"
            if (isConnected) {
              MIDI.sendNoteOff(note, velocity, 1);
            }
          } else {
            noteOn(note, velocity);
            if (isConnected) {
              MIDI.sendNoteOn(note, velocity, 1);
            }
          }
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
