async function connectToDevice() {
    const deviceHostname = 'pianolux.local';
    const deviceInfo = {
        name: 'PianoLux Device',
        address: deviceHostname
    };
    // Show the device info in the console
    console.log('Connected to device:', deviceInfo.name, 'at', deviceInfo.address);

    // Show the popup with device info
    showPopup('ESP32 Device IP: ' + deviceHostname);
}

function showPopup(deviceInfo) {
    const popup = document.getElementById('popup');
    const deviceInfoElement = document.getElementById('deviceInfo');
    const placeholderLink = document.getElementById('placeholderLink');

    deviceInfoElement.innerText = deviceInfo;
    placeholderLink.href = 'http://' + deviceInfo.replace('ESP32 Device IP: ', '');

    // Show the popup
    popup.style.display = 'block';
}