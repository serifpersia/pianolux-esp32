<div align="center">

![pianolux_logo](https://github.com/serifpersia/pianolux-esp32/assets/62844718/41b64e47-2d2b-4114-b2ca-05d3ef084215)

<h1><span class="piano-text" style="color: white;">PIANO</span><span class="lux-text" style="color: yellow;">Lux</span></h1>   

[![Release](https://img.shields.io/github/release/serifpersia/pianolux-esp32.svg?style=flat-square)](https://github.com/serifpersia/pianolux-esp32/releases)
[![License](https://img.shields.io/github/license/serifpersia/pianolux-esp32?color=blue&style=flat-square)](https://raw.githubusercontent.com/serifpersia/pianolux-esp32/master/LICENSE)
[![Discord](https://img.shields.io/discord/1077195120950120458.svg?colorB=blue&label=discord&style=flat-square)](https://discord.gg/MAypyD7k86)
</div>

## Demo
<div align="center">
   
[Check out the demo here](https://github.com/serifpersia/pianolux-esp32/assets/62844718/48c77c5e-b7bd-4edb-aa62-6e316cbeebec)
</div>

**PianoLux-ESP32** is a straightforward web-based interface for controlling a WS2812 5V LED Strip with a USB MIDI Piano.Supports USB,Bluetooth & Wifi MIDI source. You can easily configure effects, colors, and parameters through a locally hosted web server on the ESP32 board.
For support, questions, or suggestions, join our Discord Server:

[![Discord Server](https://discordapp.com/api/guilds/1077195120950120458/widget.png?style=banner2)](https://discord.gg/MAypyD7k86)
## Supported Boards
- ESP32-S2 dev board
- ESP32-S3 dev board
- *Any WiFi capable ESP32 board can be used with MIDI over network
- Bluetooth MIDI currently supported on ESPS3 & ESP32

## Supported LED Strips
- WS2812 5V 144/m
- WS2812 5V 72/m (1:1 ratio)
- *88/76 Keys need 176 LEDs of 144 LEDs/m density, so more than 1m of strip is needed. Get a 2m strip and cut the excess (after the 176th LED).

## Modes Of Operation
- Default: First time install Portal AP, after WiFi Network configure ESP32 board starts in STA_MODE(uses local network configured for connection).
- AP: Connecting pin 12 to GND pin sets ESP32 board to be used in Access Point mode, WiFi devices connect directly to this non-password protected WiFi AP. 
PianoLux web interface can be accessed using 192.168.1.1 IP Address after connecting to the AP WiFi.
- Portal AP: Connecting pin 10 to GND pin sets ESP32 to be back into setup WiFiManager Captive Portal, generally useful for deleting setup network and setting new network.
Make sure you disconnect jumper wire to use default mode after finishing re configuring your network setup using this Portal AP mode!

## Installation
Use Auto install page to automatically install PianoLux firmware on your board with one click.
- ESP32,ESP32-S2 & ESP32-S3 with 4MB/8MB/16MB flash memory supported
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
  
## Setup
After auto or manual installation. Connect to ESP32's Access Point WiFi. If your WiFi capable device didn't redirect you to WiFiManager's captive portal,
go to it manually by typing 192.168.4.1
Once here click on Configure Network button and connect to your local WiFi network. If you you no longer see this AP and your leds show IP address, PianoLux is ready to be used.
Simply access the web interface by IP or MDNS pianolux.local

## Read IP Address via LEDS & MDNS
Read IP from LED strip(default data pin 18)
Follow this image to read your ESP32's IP address

You can also access the web interface of PianoLux by using MDNS hostname by typing pianolux.local instead of IP.
MDNS might work depending on your network and devices. 
Most should be able to connect but you can always use IP route instead

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

## Power
To power esp32 and indirectly led strip you either have to use spare usb port labled COM in s3 case or use any available usb port on your esp32 board if your usb midi connetion is done with direct pins.
As for more power leds should be powered from different 5v source with good amount of Amps(3A or 3000mA is optimal I wouldn't push it more) just keep in mind ground should be shared between two 5v sources, ideally
you can power both led strip and esp32 from direct 5v pin but I'm not sure if ESP32 boards can handle those currents. I suggest keeping power setup simple from usb port you have free on your board. 
USB MIDI Devices do not provide 5V so ESP32 can't be powered from USB MIDI device alone, in some cases you even have to provide 5v to USB MIDI devices that aren't selfpowered(MIDI conttrollers).

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

