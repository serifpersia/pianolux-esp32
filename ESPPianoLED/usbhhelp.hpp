/*
 * MIT License
 *
 * Copyright (c) 2021 touchgadgetdev@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

const TickType_t HOST_EVENT_TIMEOUT = 1;
const TickType_t CLIENT_EVENT_TIMEOUT = 1;

usb_host_client_handle_t Client_Handle;
usb_device_handle_t Device_Handle;
typedef void (*usb_host_enum_cb_t)(const usb_config_desc_t *config_desc);
static usb_host_enum_cb_t _USB_host_enumerate;

void _client_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg)
{
  esp_err_t err;
  switch (event_msg->event)
  {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      err = usb_host_device_open(Client_Handle, event_msg->new_dev.address, &Device_Handle);
      usb_device_info_t dev_info;
      err = usb_host_device_info(Device_Handle, &dev_info);

      const usb_device_desc_t *dev_desc;
      err = usb_host_get_device_descriptor(Device_Handle, &dev_desc);

      const usb_config_desc_t *config_desc;
      err = usb_host_get_active_config_descriptor(Device_Handle, &config_desc);
      (*_USB_host_enumerate)(config_desc);
      break;

    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      break;

    default:
      break;
  }
}

void usbh_setup(usb_host_enum_cb_t enumeration_cb)
{
  const usb_host_config_t config = {
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  esp_err_t err = usb_host_install(&config);

  const usb_host_client_config_t client_config = {
    .is_synchronous = false,
    .max_num_event_msg = 5,
    .async = {
      .client_event_callback = _client_event_callback,
      .callback_arg = Client_Handle
    }
  };
  err = usb_host_client_register(&client_config, &Client_Handle);

  _USB_host_enumerate = enumeration_cb;
}

void usbh_task(void)
{
  uint32_t event_flags;
  static bool all_clients_gone = false;
  static bool all_dev_free = false;

  esp_err_t err = usb_host_lib_handle_events(HOST_EVENT_TIMEOUT, &event_flags);
  if (err == ESP_OK) {
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      all_clients_gone = true;
    }
    if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
      all_dev_free = true;
    }
  }
  else {
    if (err != ESP_ERR_TIMEOUT) {
    }
  }

  err = usb_host_client_handle_events(Client_Handle, CLIENT_EVENT_TIMEOUT);
  if ((err != ESP_OK) && (err != ESP_ERR_TIMEOUT)) {
  }
}
