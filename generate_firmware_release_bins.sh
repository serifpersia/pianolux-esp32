#!/bin/bash
# generate_firmware_release_bins.sh

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FIRMWARE_DIR=""
RELEASE_DIR="$SCRIPT_DIR/release"
FIRMWARES_DIR="$SCRIPT_DIR/firmwares"

# Function to get board type from user
getBoardType() {
  echo "Select board type:"
  echo "1. ESP32"
  echo "2. ESP32 S2"
  echo "3. ESP32 S3"
  read -p "Enter board type (1-3): " board_choice

  case $board_choice in
    1) board_type="esp32" ;;
    2) board_type="esp32s2" ;;
    3) board_type="esp32s3" ;;
    *) echo -e "${RED}Invalid board type selection. Please enter a number from 1 to 3.${NC}"
       getBoardType ;;
  esac
}

# Function to get flash size from user
getFlashSize() {
  echo
  echo "Select flash size:"
  echo "1. 4MB"
  echo "2. 8MB"
  echo "3. 16MB"
  echo "4. 32MB"
  read -p "Enter flash size (1-4): " flash_choice

  case $flash_choice in
    1)
      flash_size="4MB"
      flash_size_display="4mb"
      ;;
    2)
      flash_size="8MB"
      flash_size_display="8mb"
      ;;
    3)
      flash_size="16MB"
      flash_size_display="16mb"
      ;;
    4)
      flash_size="32MB"
      flash_size_display="32mb"
      ;;
    *)
      echo -e "${RED}Invalid flash size selection. Please enter a number from 1 to 4.${NC}"
      getFlashSize
      ;;
  esac
}

# Function to ask user for firmware directory path and validate files
getFirmwareDirectory() {
  while true; do
    read -p "Enter the directory path for your firmware files: " firmware_dir

    if [ ! -d "$firmware_dir" ]; then
      echo -e "${RED}Error: The specified directory does not exist.${NC}"
      continue
    fi

    # Find bootloader bin file
    bootloader_bin=$(find "$firmware_dir" -maxdepth 1 -name "*.ino.bootloader.bin" | head -n 1)
    if [ -z "$bootloader_bin" ]; then
      echo -e "${RED}Error: .ino.bootloader.bin not found in specified directory.${NC}"
      continue
    fi

    # Find app bin file
    app_bin=$(find "$firmware_dir" -maxdepth 1 -name "*.ino.bin" | head -n 1)
    if [ -z "$app_bin" ]; then
      echo -e "${RED}Error: .ino.bin not found in specified directory.${NC}"
      continue
    fi

    # Find partitions bin file
    partitions_bin=$(find "$firmware_dir" -maxdepth 1 -name "*.ino.partitions.bin" | head -n 1)
    if [ -z "$partitions_bin" ]; then
      echo -e "${RED}Error: .ino.partitions.bin not found in specified directory.${NC}"
      continue
    fi

    # Find spiffs bin file
    spiffs_bin=$(find "$firmware_dir" -maxdepth 1 -name "*.spiffs.bin" | head -n 1)
    if [ -z "$spiffs_bin" ]; then
      echo -e "${RED}Error: .spiffs.bin not found in specified directory.${NC}"
      continue
    fi

    break
  done
}

# Function to generate and execute commands
generateCommands() {
  # Find esptool.py
  esptool_py=$(find "$HOME/.arduino15/packages/esp32/tools/esptool_py" -name "esptool.py" | head -n 1)
  if [ -z "$esptool_py" ]; then
    echo -e "${RED}Error: esptool.py not found in ~/.arduino15/packages/esp32/tools/esptool_py${NC}"
    echo "Please ensure the ESP32 Arduino core is installed."
    exit 1
  fi

  # Find boot_app0.bin
  boot_bin_dir=$(find "$HOME/.arduino15/packages/esp32/hardware/esp32" -path "*/tools/partitions" -type d | head -n 1)
  if [ -z "$boot_bin_dir" ]; then
    echo -e "${RED}Error: partitions tools directory not found.${NC}"
    echo "Please ensure the ESP32 Arduino core is installed."
    exit 1
  fi

  if [ ! -f "$boot_bin_dir/boot_app0.bin" ]; then
    echo -e "${RED}Error: boot_app0.bin not found at $boot_bin_dir${NC}"
    exit 1
  fi

  # Set offsets
  if [ "$board_type" == "esp32s3" ]; then
    bootloader_bin_offset="0x0"
  else
    bootloader_bin_offset="0x1000"
  fi

  boot_bin_offset="0xe000"
  partitions_bin_offset="0x8000"
  app_bin_offset="0x10000"
  spiffs_bin_offset="0x310000"

  # Set board_name_flash based on board_type and flash_size
  board_name_flash="${board_type}_${flash_size_display}"

  # Construct the esptool command
  esptool_command=(
    python3 "$esptool_py"
    --chip "$board_type"
    merge_bin
    -o "$board_name_flash.bin"
    --flash_mode dio
    --flash_freq 40m
    --flash_size "$flash_size"
    "$bootloader_bin_offset" "$bootloader_bin"
    "$partitions_bin_offset" "$partitions_bin"
    "$boot_bin_offset" "$boot_bin_dir/boot_app0.bin"
    "$app_bin_offset" "$app_bin"
    "$spiffs_bin_offset" "$spiffs_bin"
  )

  echo
  echo -e "${GREEN}The following command will be executed:${NC}"
  echo "${esptool_command[*]}"
  echo

  read -p "Press enter to execute the command..."

  # Execute the esptool command
  "${esptool_command[@]}"

  if [ $? -ne 0 ]; then
    echo -e "${RED}Error: esptool command failed.${NC}"
    exit 1
  fi
}

# Function to create required directories
createDirectories() {
  mkdir -p "$FIRMWARES_DIR"
  mkdir -p "$RELEASE_DIR"
}

# Function to copy and rename the .bin files
copyAndRenameFiles() {
  if [ -f "$app_bin" ]; then
    cp "$app_bin" "$RELEASE_DIR/app.bin"
    case $board_type in
      "esp32")
        mv "$RELEASE_DIR/app.bin" "$RELEASE_DIR/esp32_firmware.bin"
        ;;
      "esp32s2")
        mv "$RELEASE_DIR/app.bin" "$RELEASE_DIR/esp32s2_firmware.bin"
        ;;
      "esp32s3")
        mv "$RELEASE_DIR/app.bin" "$RELEASE_DIR/esp32s3_firmware.bin"
        ;;
    esac
    echo -e "${GREEN}Copied app firmware to release directory.${NC}"
  else
    echo -e "${RED}Error: App bin file '$app_bin' does not exist.${NC}"
  fi

  if [ -f "$spiffs_bin" ]; then
    cp "$spiffs_bin" "$RELEASE_DIR/spiffs.bin"
    case $board_type in
      "esp32")
        mv "$RELEASE_DIR/spiffs.bin" "$RELEASE_DIR/esp32_filesystem.bin"
        ;;
      "esp32s2")
        mv "$RELEASE_DIR/spiffs.bin" "$RELEASE_DIR/esp32s2_filesystem.bin"
        ;;
      "esp32s3")
        mv "$RELEASE_DIR/spiffs.bin" "$RELEASE_DIR/esp32s3_filesystem.bin"
        ;;
    esac
    echo -e "${GREEN}Copied spiffs filesystem to release directory.${NC}"
  else
    echo -e "${RED}Error: Spiffs bin file '$spiffs_bin' does not exist.${NC}"
  fi

  # Move the merged firmware to firmwares directory
  if [ -f "$board_name_flash.bin" ]; then
    mv "$board_name_flash.bin" "$FIRMWARES_DIR/"
    echo -e "${GREEN}Moved merged firmware to firmwares directory.${NC}"
  else
    echo -e "${RED}Error: Merged firmware '$board_name_flash.bin' was not created.${NC}"
  fi
}

# Main script flow
echo -e "${GREEN}=== PianoLux Firmware Generator (Linux) ===${NC}"
echo

getBoardType
getFlashSize
getFirmwareDirectory
generateCommands
createDirectories
copyAndRenameFiles

echo
echo -e "${GREEN}=== Done! ===${NC}"
echo "Firmware files are in: $RELEASE_DIR"
echo "Merged firmware is in: $FIRMWARES_DIR"
read -p "Press enter to exit..."
