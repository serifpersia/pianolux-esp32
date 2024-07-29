@echo off
rem Libraries zip links are provided below for non windows installation of Arduino IDE 1.8.x please manually download and extract to your Arduino libraries folder

rem Set the Arduino libraries directory
set ARDUINO_LIB_DIR=%USERPROFILE%\Documents\Arduino\libraries
set ARDUINO_TOOLS_DIR=%USERPROFILE%\Documents\Arduino\tools

echo Installing libraries to %ARDUINO_LIB_DIR%...

rem Create a temporary directory
mkdir arduino_temp
cd arduino_temp

rem Install AppleMIDI library
echo Installing AppleMIDI library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/lathoub/Arduino-AppleMIDI-Library/archive/refs/heads/master.zip', 'AppleMIDI.zip')"
powershell -command "Expand-Archive -Path AppleMIDI.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ArduinoJson library
echo Installing ArduinoJson library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/bblanchon/ArduinoJson/archive/refs/heads/master.zip', 'ArduinoJson.zip')"
powershell -command "Expand-Archive -Path ArduinoJson.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ArduinoMIDI library
echo Installing ArduinoMIDI library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/FortySevenEffects/arduino_midi_library/archive/refs/heads/master.zip', 'ArduinoMIDI.zip')"
powershell -command "Expand-Archive -Path ArduinoMIDI.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ArduinoOTA library
echo Installing ArduinoOTA library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/JAndrassy/ArduinoOTA/archive/refs/tags/1.1.0.zip', 'ArduinoOTA.zip')"
powershell -command "Expand-Archive -Path ArduinoOTA.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ElegantOTA library
echo Installing ElegantOTA library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/ayushsharma82/ElegantOTA/archive/refs/heads/master.zip', 'AsyncElegantOTA.zip')"
powershell -command "Expand-Archive -Path AsyncElegantOTA.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install AsyncTCP library
echo Installing AsyncTCP library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/mathieucarbou/AsyncTCP/archive/refs/heads/master.zip', 'AsyncTCP.zip')"
powershell -command "Expand-Archive -Path AsyncTCP.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ESP32-BLE-MIDI library
echo Installing ESP32-BLE-MIDI library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/max22-/ESP32-BLE-MIDI/archive/refs/tags/v0.3.2.zip', 'ESP32-BLE-MIDI.zip')"
powershell -command "Expand-Archive -Path ESP32-BLE-MIDI.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install ESP32PartitionTool plugin
echo Installing ESP32PartitionTool plugin...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/serifpersia/esp32partitiontool/releases/latest/download/ESP32PartitionTool-Arduino.zip', 'ESP32PartitionTool.zip')"
powershell -command "Expand-Archive -Path ESP32PartitionTool.zip -DestinationPath %ARDUINO_TOOLS_DIR% -Force"

rem Install ESPAsyncWebServer library
echo Installing ESPAsyncWebServer library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/mathieucarbou/ESPAsyncWebServer/archive/refs/heads/master.zip', 'ESPAsyncWebServer.zip')"
powershell -command "Expand-Archive -Path ESPAsyncWebServer.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install FastLED library
echo Installing FastLED library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/FastLED/FastLED/archive/refs/heads/master.zip', 'FastLED.zip')"
powershell -command "Expand-Archive -Path FastLED.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install NimBLE-Arduino library
echo Installing NimBLE-Arduino library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/h2zero/NimBLE-Arduino/archive/refs/tags/1.4.2.zip', 'NimBLE-Arduino.zip')"
powershell -command "Expand-Archive -Path NimBLE-Arduino.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install WebSockets library
echo Installing WebSockets library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/Links2004/arduinoWebSockets/archive/refs/heads/master.zip', 'WebSockets.zip')"
powershell -command "Expand-Archive -Path WebSockets.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Install WiFiManager library
echo Installing WiFiManager library...
powershell -command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/tzapu/WiFiManager/archive/refs/heads/master.zip', 'WiFiManager.zip')"
powershell -command "Expand-Archive -Path WiFiManager.zip -DestinationPath %ARDUINO_LIB_DIR% -Force"

rem Delete the temporary directory
cd..
rmdir /s /q arduino_temp

echo Libraries installed successfully.

rem Pause to keep the command prompt window open (optional)
pause
