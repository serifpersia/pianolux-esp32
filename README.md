<div align="center">
  <img src="https://github.com/serifpersia/pianoled-esp32/assets/62844718/4aeb819a-cbd7-4347-891b-b957f1046b6d">
  
  [![Release](https://img.shields.io/github/release/serifpersia/pianoled-esp32.svg?style=flat-square)](https://github.com/serifpersia/pianoled-esp32/releases)
  [![License](https://img.shields.io/github/license/serifpersia/pianoled-esp32?color=blue&style=flat-square)](https://raw.githubusercontent.com/serifpersia/pianoled-esp32/master/LICENSE)
  [![Discord](https://img.shields.io/discord/1077195120950120458.svg?colorB=blue&label=discord&style=flat-square)](https://discord.gg/S6xmuX4Hx5)
</div>

## Demo
<div align="center">

[![PianoLED Demo](https://github.com/serifpersia/pianoled-arduino/assets/62844718/a2b7f978-be6d-4da2-badf-6ee7dd0ce6f6)](https://www.youtube.com/watch?v=giaSAFUxdw8)

</div>

**PianoLED-ESP32** is a simple web-based interface that allows you to control a WS2812 5V LED Strip with a USB MIDI Piano. You can configure all effects, colors, and parameters through a locally hosted webserver on the ESP32 board.

## Supported Boards

- ESP32-S2
- ESP32-S3
- *Any WiFi capable ESP32 board can be used with MIDI over network
- *Support for Bluetooth MIDI is planned

## Supported LED Strips

- WS2812 5V 144/m
- WS2812 5V 72/m (1:1 ratio)
- *88/76 Keys need 176 leds of 144leds/m density so more than 1m of strip is needed, get 2m one and cut the excess(after 176th led).

## Installation

Go to the PianoLED [website](https://serifpersia.github.io/pianoled-esp32/install.html) to install PianoLED on ESP32 S2/S3 with one button.
You can find your ESP32 S3/S3 IP via the website ViewIP or reading it from leds or from your router's DHCP clients page

Join Discord Server for any help, questions or suggestions: 

<a href="https://discord.gg/S6xmuX4Hx5"><img src="https://discordapp.com/api/guilds/1077195120950120458/widget.png?style=banner2" width="25%"></a>

![image](https://github.com/serifpersia/pianoled-esp32/assets/62844718/91beaa8e-c168-46cb-b048-daac8cc76df6)

This project also supports MIDI over local network so you can use midi device on your pc with this project there is no latency. For Windows use rtpMIDI software,use esp's ip and port 5004. ESP must be in non AP mode to be able to use MIDI over network

![image](https://github.com/serifpersia/pianoled-esp32/assets/62844718/607b969f-22e1-47f6-ab7a-4f76f3074b41)

### Hardware

To get started with PianoLED on the ESP platform, you have two options to choose from:

#### Option 1: ESP32-S2 Single or Dual USB Port Dev Board

- Create a DIY USB OTG/Female USB cable:
  - Cut down a short(under 15cm) USB cable with a female port on one end.
  - Splice 4 female-end jumper wires with the 4 USB wires, matching USB wire colors with jumper cable colors.
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

If you use usb otg port on s2/s3 you might have esp power issue, to fix this bypass the usb otg port and do the 
diy usb micro b/c to Female A port short cable and connect wires directly to esp32's 5V,GND, 19 &  20 pins.

![image](https://github.com/serifpersia/pianoled-esp32/assets/62844718/cea8ebeb-09c5-46e9-a028-67c5447ad0f3)


![image](https://github.com/serifpersia/pianoled-esp32/assets/62844718/9ea3a1e8-52e6-40e1-9069-58c918e9e6ef)


For power, you can use the USB port of the board. The default current limit is set to 450mA. If the plan is to power the LED STRIP separately from the same or different 5V DC supply, make sure both power source grounds are connected to the ground of the ESP32 Board.

#### Option 2: ESP32-S3 Dual USB Port Dev Board

This is more of a plug & play setup. Depending on whether your board has pre-soldered pin headers and USB-OTG pads soldered, you have to bridge the USB-OTG pads,for boards without this feature, for them to work with your USB MIDI Device you need to connect the 5V pin to 5v wire of your USB cable connected to the USB OTG/Host capable port. This applies for S2 as well,some S2 boards can also have 2 usb ports one for USB Host but you may need to use 5v Pin if MIDI device doesn't function.

The same LED scheme applies to ESP-32S3. A spare COM USB port can be used for basic power, but if you need brighter LEDs, consider powering the LED strip externally.

![image](https://github.com/serifpersia/pianoled-esp32/assets/62844718/a089640f-113e-47b1-8c88-8e38e4728295)

