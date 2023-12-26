<div align="center">

![image](https://github.com/serifpersia/pianolux-esp32/assets/62844718/df9e1fc6-df19-40de-a04f-9b182e3790de)

[![Release](https://img.shields.io/github/release/serifpersia/pianolux-esp32.svg?style=flat-square)](https://github.com/serifpersia/pianolux-esp32/releases)
[![License](https://img.shields.io/github/license/serifpersia/pianolux-esp32?color=blue&style=flat-square)](https://raw.githubusercontent.com/serifpersia/pianolux-esp32/master/LICENSE)
[![Discord](https://img.shields.io/discord/1077195120950120458.svg?colorB=blue&label=discord&style=flat-square)](https://discord.gg/MAypyD7k86)
</div>

## Demo
<div align="center">
   
[Check out the demo here](https://github.com/serifpersia/pianolux-esp32/assets/62844718/48c77c5e-b7bd-4edb-aa62-6e316cbeebec)
</div>

**PianoLux-ESP32** is a straightforward web-based interface for controlling a WS2812 5V LED Strip with a USB MIDI Piano. You can easily configure effects, colors, and parameters through a locally hosted web server on the ESP32 board.
For support, questions, or suggestions, join our Discord Server:

[![Discord Server](https://discordapp.com/api/guilds/1077195120950120458/widget.png?style=banner2)](https://discord.gg/MAypyD7k86)
## Supported Boards
- ESP32-S2 dev board
- ESP32-S3 dev board
- *Any WiFi capable ESP32 dev board can be used with MIDI over network & planned bluetooth midi
- *Support for Bluetooth MIDI is planned

## Supported LED Strips
- WS2812 5V 144/m
- WS2812 5V 72/m (1:1 ratio)
- *88/76 Keys need 176 LEDs of 144 LEDs/m density, so more than 1m of strip is needed. Get a 2m strip and cut the excess (after the 176th LED).

## Installation
Use Auto install page to automatically install PianoLux firmware on your board with one click.
- ESP32, S2 4MB flash size supported
- ESP32 S3 16MB flash size supported
- ESP32 4MB flash size supported
- Only Google Chrome and Edge are supported.

[![Auto Install](https://img.shields.io/badge/Auto-%20Install-blue?style=flat-square)](https://serifpersia.github.io/pianolux-esp32/)

Manual Installation

1. Install Arduino IDE and necessary libraries.
   - Import the ESP32 Arduino core, follow [these instructions](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
   - Clone the project, extract the zip, for quick(less libraries to import) install use barebones sketch.

2. Libraries found in Arduino IDE Library Manager:
   - FastLED
   - WiFiManager
   - AsyncElegantOTA

3. Libraries that need manual zip installation:
   - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
   - [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

4. Select your ESP32 dev board(esp32, esp32s2 or esp32s3). After successful upload, go through WiFi setup. Visit the ESP32's IP address to continue (pianolux.local might also work).
   - Download the latest bin files for your board from the [release](https://github.com/serifpersia/pianolux-esp32/releases/tag/latest) page and upload via ElegantOTA page.
   - Configure the LED strip data pin (default is 18).

![WiFi Setup](https://github.com/serifpersia/pianolux-esp32/assets/62844718/2f00777b-a9aa-476f-b022-fb964bd11fd5)
![image](https://github.com/serifpersia/pianolux-esp32/assets/62844718/10bee33d-2ba6-42b8-a66d-34b45768c436)

## Read IP Address via LEDS
Read IP from LED strip(default data pin 18)
Follow this image to read your ip. the format is usually xxx.xxx.x.x or xx or x x x)

![278872118-91beaa8e-c168-46cb-b048-daac8cc76df6](https://github.com/serifpersia/pianolux-esp32/assets/62844718/3bd11a11-d939-49d8-b532-466c98aa4975)

## MIDI over Local Network
This project supports MIDI over a local network, enabling the use of MIDI devices on your PC with no latency. For Windows, use rtpMIDI software, and use ESP's IP and port 5004. Ensure the ESP is in non-AP mode to use MIDI over the network. Supports sending & reciving MIDI data betweeen ESP32 board & rtpMIDI capable device like PC using rtpMIDI application.

![MIDI Setup](https://github.com/serifpersia/pianolux-esp32/assets/62844718/607b969f-22e1-47f6-ab7a-4f76f3074b41)

### Hardware
To get started with PianoLux on the ESP platform, you have two options:

#### Option 1: ESP32-S2 Single or Dual USB Port Dev Board
- Create a DIY USB OTG/Female USB cable.
- Connect this DIY cable to pins on the ESP32-S2 as follows:
  - USB - ESP32-S2 Board Pins:
    - Red Wire - 5v Pin
    - Black Wire - GND Pin
    - White Wire - 19 Pin
    - Green Wire - 20 Pin
  - LED STRIP - ESP32-S2 Board Pins:
    - Red Wire - 5v Pin
    - White Wire - GND Pin
    - Green Wire - Data Pin (default 18)

![Option 1 Setup](https://github.com/serifpersia/pianolux-esp32/assets/62844718/cea8ebeb-09c5-46e9-a028-67c5447ad0f3)
#### Option 2: ESP32-S3 Dual USB Port Dev Board
This is more of a plug & play setup. Depending on whether your board has pre-soldered pin headers and USB-OTG pads soldered, you have to bridge the USB-OTG pads.

![Option 2 Setup](https://github.com/serifpersia/pianolux-esp32/assets/62844718/a089640f-113e-47b1-8c88-8e38e4728295)

## Features
### LED Modes
- **Default Mode 🎹:**
  - Plain HSB colored playing LEDs.

- **Splash Mode 💦:**
  - Splash effect from played MIDI notes.

- **Split Mode ↔️:**
  - Split playing LEDs into two with adjustable colors.

- **Random Mode 🎲:**
  - Random hue changes with each triggered MIDI note.

- **Velocity Mode ⚡:**
  - LEDs react based on MIDI note velocity.

- **Animation Mode 🎥:**
  - Static looping LED animations with 10 options.
  - MIDI input is ignored in this mode.

### Global Controls

- **Color Control Sliders 🌈:**
  - Adjust hue, saturation, and brightness.

- **Fade Length ⏱️:**
  - Control the duration of the fade effect.

- **Background Light 💡:**
  - Toggle and adjust background LED lights.

- **Board Config Parameters ⚙️:**
  - Set max current, LED strip data pin.

- **Piano Size Configuration 🎹:**
  - Button to configure piano size.

- **MIDI to LED Map Ratios 🎵:**
  - 1:2 and 1:1 mapping options.

- **Visual Representation 🎨:**
  - Full 88-key piano keyboard for visual aid
### Global Toggles
- **FX LED 🔀:**
  - Shift LEDs at certain solder joined points on the strip

- **BG LED 🌌:**
  - Toggle background light LEDs.
  - Adjust color and brightness separately.

- **Update BG Color 🔄:**
  - Apply HSB color adjustments to background light.

- **RV LED 🔁:**
  - Reverse LED strip direction for added flexibility.
 
## License

This project is licensed under the [MIT License](LICENSE).

