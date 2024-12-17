#if BOARD_TYPE == ESP32S3 || BOARD_TYPE == ESP32S2
bool isMIDI = false;
bool isMIDIReady = false;

const size_t MIDI_IN_BUFFERS = 8;
const size_t MIDI_OUT_BUFFERS = 8;
usb_transfer_t *MIDIOut = NULL;
usb_transfer_t *MIDIIn[MIDI_IN_BUFFERS] = { NULL };

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

// Send MIDI Note On message
void sendMIDINoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (!MIDIOut || !isMIDIReady) {
    return;
  }

  uint8_t midiData[4] = {
    0x09,                   // Cable Number 0, CIN 9 (Note On)
    0x90 | (channel & 0x0F),// Note On status byte
    note & 0x7F,           // Note number (0-127)
    velocity & 0x7F        // Velocity (0-127)
  };

  memcpy(MIDIOut->data_buffer, midiData, 4);
  MIDIOut->num_bytes = 4;

  esp_err_t err = usb_host_transfer_submit(MIDIOut);
  if (err != ESP_OK) {
    ESP_LOGI("", "MIDI Note On transfer submit failed: %x", err);
    return;
  }

  if (numConnectedClients != 0) {
    sendESP32Log("USB MIDI OUT: NOTE ON Pitch: " + String(note) + " Velocity: " + String(velocity));
  }
}

// Send MIDI Note Off message
void sendMIDINoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (!MIDIOut || !isMIDIReady) {
    return;
  }

  uint8_t midiData[4] = {
    0x08,                   // Cable Number 0, CIN 8 (Note Off)
    0x80 | (channel & 0x0F),// Note Off status byte
    note & 0x7F,           // Note number (0-127)
    velocity & 0x7F        // Velocity (usually 0 for note off)
  };

  memcpy(MIDIOut->data_buffer, midiData, 4);
  MIDIOut->num_bytes = 4;

  esp_err_t err = usb_host_transfer_submit(MIDIOut);
  if (err != ESP_OK) {
    ESP_LOGI("", "MIDI Note Off transfer submit failed: %x", err);
    return;
  }

  if (numConnectedClients != 0) {
    sendESP32Log("USB MIDI OUT: NOTE OFF Pitch: " + String(note) + " Velocity: " + String(velocity));
  }
}
#endif
