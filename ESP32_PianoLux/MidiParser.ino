#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
static void midi_transfer_cb(usb_transfer_t *transfer) {
  ESP_LOGI("", "midi_transfer_cb context: %d", transfer->context);
  if (Device_Handle == transfer->device_handle) {
    int in_xfer = transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK;
    if ((transfer->status == 0) && in_xfer) {
      uint8_t *const p = transfer->data_buffer;
      for (int i = 0; i < transfer->actual_num_bytes; i += 4) {
        if ((p[i] + p[i + 1] + p[i + 2] + p[i + 3]) == 0) break; // End of valid data
        ESP_LOGI("", "midi: %02x %02x %02x %02x", p[i], p[i + 1], p[i + 2], p[i + 3]);

        // Parse USB MIDI packet
        uint8_t cin = p[i] & 0x0F;           // Code Index Number
        uint8_t statusByte = p[i + 1];       // MIDI status byte
        uint8_t data1 = p[i + 2];            // First data byte (note or controller)
        uint8_t data2 = p[i + 3];            // Second data byte (velocity or value)
        uint8_t channel = statusByte & 0x0F; // Extract channel (0-15)

        // Format parsed MIDI data for logging
        char midiString[50];
        const char* eventType;
        if ((statusByte & 0xF0) == 0x80) eventType = "Note Off";
        else if ((statusByte & 0xF0) == 0x90) eventType = "Note On";
        else if ((statusByte & 0xF0) == 0xB0) eventType = "Control Change";
        else eventType = "Other";
        snprintf(midiString, sizeof(midiString), "Ch%d %s Data1: %d Data2: %d",
                 channel, eventType, data1, data2);

        // Process MIDI messages
        switch (statusByte & 0xF0) {
          case 0x80: // Note Off
            noteOff(data1); // data1 = note number
            if (numConnectedClients != 0) {
              sendESP32Log("USB MIDI IN: NOTE OFF Pitch: " + String(data1) + " Velocity: " + String(data2));
            }
            if (isConnected) {
              //MIDI.sendNoteOff(data1, data2, channel + 1);
              //sendESP32Log("RTP MIDI Out: Note OFF " + String(data1) + " Velocity: " + String(data2));
            }
            break;

          case 0x90: // Note On
            if (data2 == 0) { // Velocity 0 treated as Note Off per MIDI spec
              noteOff(data1);
              if (numConnectedClients != 0) {
                sendESP32Log("USB MIDI IN: NOTE OFF Pitch: " + String(data1) + " Velocity: 0");
              }
              if (isConnected) {
                //MIDI.sendNoteOff(data1, 0, channel + 1);
                //sendESP32Log("RTP MIDI Out: Note OFF " + String(data1) + " Velocity: 0");
              }
            } else {
              noteOn(data1, data2); // data1 = note, data2 = velocity
              if (numConnectedClients != 0) {
                sendESP32Log("USB MIDI IN: NOTE ON Pitch: " + String(data1) + " Velocity: " + String(data2));
              }
              if (isConnected) {
                //MIDI.sendNoteOn(data1, data2, channel + 1);
                //sendESP32Log("RTP MIDI Out: Note ON " + String(data1) + " Velocity: " + String(data2));
              }
            }
            break;

          case 0xB0: // Control Change (commented out)
            if (numConnectedClients != 0) {
              sendESP32Log("USB MIDI IN: CONTROL CHANGE Controller: " + String(data1) + " Value: " + String(data2));
            }
            if (isConnected) {
              //MIDI.sendControlChange(data1, data2, channel + 1); // data1 = controller, data2 = value
              //sendESP32Log("RTP MIDI Out: CC " + String(data1) + " Value: " + String(data2));
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
