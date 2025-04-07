#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2



void check_interface_desc_MIDI(const void *p) {
  const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;

  // USB MIDI
  if ((intf->bInterfaceClass == USB_CLASS_AUDIO) && (intf->bInterfaceSubClass == 3) && (intf->bInterfaceProtocol == 0)) {
    isMIDI = true;
    ESP_LOGI("", "Claiming a MIDI device!");
    esp_err_t err = usb_host_interface_claim(Client_Handle, Device_Handle,
                    intf->bInterfaceNumber, intf->bAlternateSetting);
    if (err != ESP_OK) ESP_LOGI("", "usb_host_interface_claim failed: %x", err);
  }
}

void prepare_endpoints(const void *p) {
  const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
  esp_err_t err;

  // must be bulk for MIDI
  if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_BULK) {
    ESP_LOGI("", "Not bulk endpoint: 0x%02x", endpoint->bmAttributes);
    return;
  }
  if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
    for (int i = 0; i < MIDI_IN_BUFFERS; i++) {
      err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &MIDIIn[i]);
      if (err != ESP_OK) {
        MIDIIn[i] = NULL;
        ESP_LOGI("", "usb_host_transfer_alloc In fail: %x", err);
      } else {
        MIDIIn[i]->device_handle = Device_Handle;
        MIDIIn[i]->bEndpointAddress = endpoint->bEndpointAddress;
        MIDIIn[i]->callback = midi_transfer_cb;
        MIDIIn[i]->context = (void *)i;
        MIDIIn[i]->num_bytes = endpoint->wMaxPacketSize;
        esp_err_t err = usb_host_transfer_submit(MIDIIn[i]);
        if (err != ESP_OK) {
          ESP_LOGI("", "usb_host_transfer_submit In fail: %x", err);
        }
      }
    }
  } else {
    err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &MIDIOut);
    if (err != ESP_OK) {
      MIDIOut = NULL;
      ESP_LOGI("", "usb_host_transfer_alloc Out fail: %x", err);
      return;
    }
    ESP_LOGI("", "Out data_buffer_size: %d", MIDIOut->data_buffer_size);
    MIDIOut->device_handle = Device_Handle;
    MIDIOut->bEndpointAddress = endpoint->bEndpointAddress;
    MIDIOut->callback = midi_transfer_cb;
    MIDIOut->context = NULL;
    //    MIDIOut->flags |= USB_TRANSFER_FLAG_ZERO_PACK;
  }
  isMIDIReady = ((MIDIOut != NULL) && (MIDIIn[0] != NULL));
}

void show_config_desc_full(const usb_config_desc_t *config_desc) {
  // Full decode of config desc.
  const uint8_t *p = &config_desc->val[0];
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength) {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength) {
      const uint8_t bDescriptorType = *(p + 1);
      switch (bDescriptorType) {
        case USB_B_DESCRIPTOR_TYPE_CONFIGURATION:
          break;
        case USB_B_DESCRIPTOR_TYPE_INTERFACE:
          if (!isMIDI) check_interface_desc_MIDI(p);
          break;
        case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
          if (isMIDI && !isMIDIReady) {
            prepare_endpoints(p);
          }
          break;
        default:
          break;
      }
    } else {
      return;
    }
  }
}

// Helper function to queue a message
static bool queueMidiMessage(uint8_t status, uint8_t channel, uint8_t data1, uint8_t data2) {
  if (!isMIDIReady || midiOutQueue == NULL) {
    return false; // Don't queue if USB not ready or queue not created
  }

  MidiMessage msg;
  msg.status = status;
  msg.channel = channel;
  msg.data1 = data1;
  msg.data2 = data2;

  // Try to send to the queue, wait briefly if full (optional, 0 means no wait)
  if (xQueueSend(midiOutQueue, &msg, pdMS_TO_TICKS(10)) != pdPASS) {
    ESP_LOGW("", "MIDI Out Queue full!");
    // Optionally drop oldest message: xQueueOverwrite(midiOutQueue, &msg);
    return false; // Indicate failure to queue
  }
  return true; // Successfully queued
}

// Send MIDI Note On message (Modified)
void sendUSBMIDINoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (numConnectedClients != 0 && CLIENT_LOGGER)
  {
    sendESP32Log("USB MIDI PLAYER Out: Note ON " + String(note) + " Velocity: " + String(velocity));
  }
  queueMidiMessage(0x90, channel, note, velocity);
}

// Send MIDI Note Off message (Modified)
void sendUSBMIDINoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (numConnectedClients != 0 && CLIENT_LOGGER)
  {
    sendESP32Log("USB MIDI PLAYER Out: Note OFF " + String(note) + " Velocity: " + String(velocity));
  }
  queueMidiMessage(0x80, channel, note, velocity);
}

// Modify other sendUSBMIDI... functions similarly if you use them
void sendUSBMIDIControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  if (numConnectedClients != 0 && CLIENT_LOGGER)
  {
    sendESP32Log("USB MIDI PLAYER Out: CC " + String(controller) + " Value: " + String(value));
  }
  queueMidiMessage(0xB0, channel, controller, value);
}

//void sendUSBMIDIProgramChange(uint8_t channel, uint8_t program) {
//  queueMidiMessage(0xC0, channel, program, 0); // data2 is unused for PC
//}
//
//// Pitch bend needs slight adaptation for 14-bit value
//void sendUSBMIDIPitchBend(uint8_t channel, int bendValue) {
//  // Convert signed bendValue (-8192 to +8191) to unsigned 14-bit (0-16383)
//  uint16_t unsignedBend = (bendValue + 8192) & 0x3FFF; // Ensure 14-bit range
//  uint8_t lsb = unsignedBend & 0x7F;         // Least Significant 7 bits
//  uint8_t msb = (unsignedBend >> 7) & 0x7F;  // Most Significant 7 bits
//  queueMidiMessage(0xE0, channel, lsb, msb);
//}
#endif
