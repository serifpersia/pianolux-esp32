#!/bin/bash

# Define the Arduino libraries and tools directories
ARDUINO_LIB_DIR="$HOME/Arduino/libraries"
ARDUINO_TOOLS_DIR="$HOME/Arduino/tools"

# Create the temporary directory
mkdir -p arduino_temp
cd arduino_temp

# Function to download and extract libraries
install_library() {
    local url="$1"
    local zip_name="$2"
    local dest_dir="$3"

    echo "Installing $zip_name library..."
    curl -L "$url" -o "$zip_name"
    unzip "$zip_name" -d "$dest_dir"
}

# Install libraries
install_library "https://github.com/lathoub/Arduino-AppleMIDI-Library/archive/refs/heads/master.zip" "AppleMIDI.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/bblanchon/ArduinoJson/archive/refs/heads/master.zip" "ArduinoJson.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/FortySevenEffects/arduino_midi_library/archive/refs/heads/master.zip" "ArduinoMIDI.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/JAndrassy/ArduinoOTA/archive/refs/tags/1.1.0.zip" "ArduinoOTA.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/ayushsharma82/ElegantOTA/archive/refs/heads/master.zip" "AsyncElegantOTA.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/mathieucarbou/AsyncTCP/archive/refs/heads/master.zip" "AsyncTCP.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/max22-/ESP32-BLE-MIDI/archive/refs/tags/v0.3.2.zip" "ESP32-BLE-MIDI.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/serifpersia/esp32partitiontool/releases/latest/download/ESP32PartitionTool-Arduino.zip" "ESP32PartitionTool.zip" "$ARDUINO_TOOLS_DIR"
install_library "https://github.com/mathieucarbou/ESPAsyncWebServer/archive/refs/heads/master.zip" "ESPAsyncWebServer.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/FastLED/FastLED/archive/refs/heads/master.zip" "FastLED.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/h2zero/NimBLE-Arduino/archive/refs/tags/1.4.2.zip" "NimBLE-Arduino.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/Links2004/arduinoWebSockets/archive/refs/heads/master.zip" "WebSockets.zip" "$ARDUINO_LIB_DIR"
install_library "https://github.com/tzapu/WiFiManager/archive/refs/heads/master.zip" "WiFiManager.zip" "$ARDUINO_LIB_DIR"

# Cleanup
cd ..
rm -rf arduino_temp

echo "Libraries installed successfully."
