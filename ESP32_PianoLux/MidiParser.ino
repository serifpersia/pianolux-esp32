#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
static void midi_transfer_cb(usb_transfer_t *transfer) {
  // Check if the callback is for our device
  if (Device_Handle != transfer->device_handle) {
    ESP_LOGW("", "midi_transfer_cb for wrong device handle");
    // Optional: Handle freeing the transfer if it was allocated outside our known ones?
    return;
  }

  // Check if it's an IN transfer (MIDI data received from USB device)
  if (transfer->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
    // This is an IN transfer (like MIDIIn[...])
    if (transfer->status == USB_TRANSFER_STATUS_COMPLETED) {
      uint8_t *const p = transfer->data_buffer;
      //ESP_LOGI("", "MIDI IN Transfer Complete (%d bytes)", transfer->actual_num_bytes);
      for (int i = 0; i < transfer->actual_num_bytes; i += 4) {
        // Basic validity check
        if (p[i] == 0 && p[i + 1] == 0 && p[i + 2] == 0 && p[i + 3] == 0 && i > 0) {
          //ESP_LOGD("", "Skipping padding zeros in MIDI IN");
          break;
        }
        if ((p[i] & 0x0F) == 0) { // CIN 0 is reserved/invalid in USB MIDI 1.0
          //ESP_LOGD("", "Skipping invalid CIN 0 in MIDI IN");
          continue;
        }

        //ESP_LOGI("", "midi IN raw: %02x %02x %02x %02x", p[i], p[i + 1], p[i + 2], p[i + 3]);

        // Parse USB MIDI packet
        uint8_t cin = p[i] & 0x0F;
        uint8_t statusByte = p[i + 1];
        uint8_t data1 = p[i + 2];
        uint8_t data2 = p[i + 3];
        uint8_t channel = statusByte & 0x0F;

        // --- Process MIDI IN messages ---
        switch (statusByte & 0xF0) {
          case 0x80: // Note Off
            noteOff(data1);
            break;
          case 0x90: // Note On
            if (data2 == 0) {
              noteOff(data1);
            } else {
              noteOn(data1, data2);
            }
            break;
          case 0xB0: // Control Change
            // Handle CC if needed
            break;
            // Add other cases (Prog Change, Pitch Bend, etc.) if needed
        }
        // --- End MIDI IN processing ---
      }

      // Re-submit the IN transfer buffer to continue listening
      esp_err_t err = usb_host_transfer_submit(transfer);
      if (err != ESP_OK) {
        ESP_LOGE("", "usb_host_transfer_submit IN failed: %s", esp_err_to_name(err));
        // Consider how to recover - maybe re-allocate and re-submit?
      }
    } else {
      ESP_LOGW("", "MIDI IN transfer failed, status: %d", transfer->status);
      // Handle IN transfer errors (e.g., STALL, ERROR)
      // Maybe try re-submitting? Or log and stop?
      // Attempt re-submission cautiously
      vTaskDelay(pdMS_TO_TICKS(10)); // Small delay before retry
      esp_err_t err = usb_host_transfer_submit(transfer);
      if (err != ESP_OK) {
        ESP_LOGE("", "Retry usb_host_transfer_submit IN failed: %s", esp_err_to_name(err));
      }
    }
  } else {
    // This is an OUT transfer (MIDIOut completion)
    if (transfer == MIDIOut) {
      //ESP_LOGD("", "MIDI OUT Transfer Complete, Status: %d", transfer->status);
      if (transfer->status != USB_TRANSFER_STATUS_COMPLETED) {
        ESP_LOGW("", "MIDI OUT transfer failed, status: %d", transfer->status);
        // Handle potential OUT errors if necessary
      }
      // Mark the OUT buffer as free, regardless of status for now
      // (the sending logic will handle retries if needed)
      midiOutBusy = false;
    } else {
      ESP_LOGW("", "Callback received for unknown OUT transfer");
    }
  }
}
#endif
