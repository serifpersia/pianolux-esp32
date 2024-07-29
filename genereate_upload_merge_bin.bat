@echo off
cls

rem Main script flow
rem Call the functions to get user input
call :getBoardType
call :getFlashSize

rem Call the function to get firmware directory path and validate files
call :getFirmwareDirectory

rem Call the function to generate and execute commands
call :generateCommands

rem Ensure the required directories exist
call :createDirectories

rem Move the generated .bin file to the firmwares directory
move "%esptool_dir%\%board_name_flash%.bin" "%~dp0\firmwares\%board_name_flash%.bin"

rem Copy and rename the .bin files to the release directory
call :copyAndRenameFiles

rem End of script
pause
exit /b

rem Function to ask user for firmware directory path and validate files
:getFirmwareDirectory
set /p "firmware_dir=Enter the directory path for your firmware files: "

rem Validate directory existence
if not exist "%firmware_dir%" (
    echo Error: The specified directory does not exist.
    goto :getFirmwareDirectory
)

rem Find bootloader bin file
for %%f in ("%firmware_dir%\*.ino.bootloader.bin") do (
    set "bootloader_bin=%%~f"
    goto :found_bootloader
)
echo Error: .ino.bootloader.bin not found in specified directory.
goto :getFirmwareDirectory
:found_bootloader

rem Find app bin file
for %%f in ("%firmware_dir%\*.ino.bin") do (
    set "app_bin=%%~f"
    goto :found_app_bin
)
echo Error: .ino.bin not found in specified directory.
goto :getFirmwareDirectory
:found_app_bin

rem Find partitions bin file
for %%f in ("%firmware_dir%\*.ino.partitions.bin") do (
    set "partitions_bin=%%~f"
    goto :found_partitions_bin
)
echo Error: .ino.partitions.bin not found in specified directory.
goto :getFirmwareDirectory
:found_partitions_bin

rem Find spiffs bin file
for %%f in ("%firmware_dir%\*.spiffs.bin") do (
    set "spiffs_bin=%%~f"
    goto :found_spiffs_bin
)
echo Error: .spiffs.bin not found in specified directory.
goto :getFirmwareDirectory
:found_spiffs_bin

exit /b

rem Function to ask user for board type input
:getBoardType
rem Display instructions for board type
echo Select board type:
echo 1. ESP32
echo 2. ESP32 S2
echo 3. ESP32 S3
set /p "board_choice=Enter board type (1-3): "

rem Validate board type input and set board_type variable
if "%board_choice%"=="1" (
    set "board_type=esp32"
) else if "%board_choice%"=="2" (
    set "board_type=esp32s2"
) else if "%board_choice%"=="3" (
    set "board_type=esp32s3"
) else (
    echo Invalid board type selection. Please enter a number from 1 to 3.
    call :getBoardType
)
exit /b

rem Function to ask user for flash size input
:getFlashSize
rem Display instructions for flash size
echo.
echo Select flash size:
echo 1. 4MB
echo 2. 8MB
echo 3. 16MB
echo 4. 32MB
set /p "flash_choice=Enter flash size (1-4): "

rem Validate flash size input and set flash_size variable
if "%flash_choice%"=="1" (
    set "flash_size=4MB"
    set "flash_size_display=4mb"
) else if "%flash_choice%"=="2" (
    set "flash_size=8MB"
    set "flash_size_display=8mb"
) else if "%flash_choice%"=="3" (
    set "flash_size=16MB"
    set "flash_size_display=16mb"
) else if "%flash_choice%"=="4" (
    set "flash_size=32MB"
    set "flash_size_display=32mb"
) else (
    echo Invalid flash size selection. Please enter a number from 1 to 4.
    call :getFlashSize
)
exit /b

rem Function to generate commands based on user inputs
:generateCommands
rem Set variables for directories and offsets
set "esptool_dir=%localappdata%\Arduino15\packages\esp32\tools\esptool_py\4.5.1"
set "boot_bin_dir=%localappdata%\Arduino15\packages\esp32\hardware\esp32\2.0.17\tools\partitions"

set "bootloader_bin_offset=0x1000"
if "%board_type%"=="esp32s3" set "bootloader_bin_offset=0x0"

set "boot_bin_offset=0xe000"
set "partitions_bin_offset=0x8000"
set "app_bin_offset=0x10000"
set "spiffs_bin_offset=0x310000"

rem Set board_name_flash based on board_type and flash_size
set "board_name_flash=%board_type%_%flash_size_display%"

rem Navigate to esptool directory
cd /d "%esptool_dir%"

rem Construct the esptool command
set "esptool_command=esptool.exe --chip %board_type% merge_bin -o "%board_name_flash%.bin" --flash_mode dio --flash_freq 40m --flash_size %flash_size% %bootloader_bin_offset% "%bootloader_bin%" %partitions_bin_offset% "%partitions_bin%" %boot_bin_offset% "%boot_bin_dir%\boot_app0.bin" %app_bin_offset% "%app_bin%" %spiffs_bin_offset% "%spiffs_bin%"

rem Print the esptool command for verification
echo.
echo The following command will be executed:
echo %esptool_command%
echo.

rem Wait for user confirmation to execute the command
pause

rem Execute the esptool command
%esptool_command%

exit /b

rem Function to create required directories
:createDirectories
if not exist "%~dp0\firmwares" (
    mkdir "%~dp0\firmwares"
)
if not exist "%~dp0\release" (
    mkdir "%~dp0\release"
)
exit /b

rem Function to copy and rename the .bin files
:copyAndRenameFiles
rem Ensure the copied files are correctly identified and copied
if exist "%app_bin%" (
    copy "%app_bin%" "%~dp0\release\app.bin"
    if "%board_type%"=="esp32" (
        ren "%~dp0\release\app.bin" "esp32_firmware.bin"
    ) else if "%board_type%"=="esp32s2" (
        ren "%~dp0\release\app.bin" "esp32s2_firmware.bin"
    ) else if "%board_type%"=="esp32s3" (
        ren "%~dp0\release\app.bin" "esp32s3_firmware.bin"
    )
) else (
    echo Error: App bin file "%app_bin%" does not exist.
)

if exist "%spiffs_bin%" (
    copy "%spiffs_bin%" "%~dp0\release\spiffs.bin"
    if "%board_type%"=="esp32" (
        ren "%~dp0\release\spiffs.bin" "esp32_filesystem.bin"
    ) else if "%board_type%"=="esp32s2" (
        ren "%~dp0\release\spiffs.bin" "esp32s2_filesystem.bin"
    ) else if "%board_type%"=="esp32s3" (
        ren "%~dp0\release\spiffs.bin" "esp32s3_filesystem.bin"
    )
) else (
    echo Error: Spiffs bin file "%spiffs_bin%" does not exist.
)

exit /b
