var Socket;

// Function to initialize WebSocket
function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

    // Add error event handler
    Socket.addEventListener('error', function (error) {
        console.error('WebSocket error:', error);
        // Handle the error here, e.g., display an error message to the user.
    });

    // Add close event handler
    Socket.addEventListener('close', function (event) {
        if (event.wasClean) {
            console.log('WebSocket closed cleanly, code=' + event.code + ', reason=' + event.reason);
        } else {
            console.error('WebSocket connection died');
            // You may want to attempt to reconnect here.
        }
    });

    // Add open event handler
    Socket.addEventListener('open', function (event) {
        console.log('WebSocket connection opened');
        sendData('RequestValues');
    });


    // Event listener to handle updates from the server
    Socket.addEventListener('message', function (event) {
        var data = JSON.parse(event.data);
        console.log('Received data from the server:', data);

        // Call the updateUI function to update the UI elements
        updateUI(data);
    });
}

// Function to send data via WebSocket with error handling
function sendData(action, data) {
    if (Socket.readyState === WebSocket.OPEN) {
        try {
            Socket.send(JSON.stringify({ action: action, ...data }));
        } catch (error) {
            console.error('Error sending data:', error);
            // Handle the error here, e.g., display an error message to the user.
        }
    } else {
        console.error('WebSocket connection not open');
        // Handle the error here, e.g., display an error message to the user or attempt to reconnect.
    }
}

function addInputListener(controlId, dataKey, factor) {
    document.getElementById(controlId).addEventListener('input', function () {
        const value = parseInt(this.value);
        if (dataKey === 'FADE') {
            const invertedValue = 255 - value;
            sendData(dataKey, { value: invertedValue });
            console.log(invertedValue);
        } else {
            sendData(dataKey, { value: value });
            console.log(value);
        }
    });
}

addInputListener('HUE', 'Hue', 1);
addInputListener('BRIGHTNESS', 'Brightness', 1);
addInputListener('FADE', 'Fade', 255);
addInputListener('SPLASH', 'Splash', 1);
addInputListener('BG', 'Background', 1);


var navbar = document.querySelector('.navbar');
var scrolling = false;

function handleScroll() {
    if (!scrolling) {
        navbar.style.opacity = '1'; // Show the navbar
        navbar.style.pointerEvents = 'auto'; // Enable pointer events
    }

    scrolling = true;

    // Use a timeout to hide the navbar after a delay when scrolling stops
    clearTimeout(window.scrollTimeout);
    window.scrollTimeout = setTimeout(function () {
        scrolling = false;
        navbar.style.opacity = '0'; // Hide the navbar
        navbar.style.pointerEvents = 'none'; // Disable pointer events
    }, 1500); // Adjust the delay time (in milliseconds) as needed
}

// Add an event listener for the scroll event
window.addEventListener('scroll', handleScroll);


function updateUI(data) {
    function updateControlValue(controlId, dataValue, maxTrack, thumb, handleMoveFunction, factor) {
        if (dataValue !== undefined) {
            document.getElementById(controlId).value = dataValue;
            const maxPosition = maxTrack.offsetWidth - thumb.offsetWidth;
            const newPosition = (dataValue / factor) * maxPosition;
            handleMoveFunction(newPosition);
        }
    }

    function updateDropdownList(dropdownId, dataValue) {
        if (dataValue !== undefined) {
            document.getElementById(dropdownId).value = dataValue;
        }
    }

    function updateToggles(controlId, dataValue) {
        if (dataValue !== undefined) {
            const checkbox = document.getElementById(controlId);
            checkbox.checked = dataValue;
        }
    }

    updateDropdownList('selected-item-modes', data.MODES);
    updateDropdownList('selected-item-animations', data.ANIMATIONS);
    updateDropdownList('selected-item-colors', data.COLORS);

    updateControlValue('HUE', data.HUE, track, thumb, handleMove, 255);
    updateControlValue('BRIGHTNESS', data.BRIGHTNESS, brightnessTrack, brightnessThumb, handleBrightnessMove, 255);
    updateControlValue('FADE', data.FADE, fadeTrack, fadeThumb, handleFadeMove, 255);
    updateControlValue('SPLASH', data.SPLASH, splashTrack, splashThumb, handleSplashMove, 16);
    updateControlValue('BG', data.BG, bgTrack, bgThumb, handleBgMove, 255);

    updateToggles('cb1-8', data.FIX_TOGGLE);
    updateToggles('cb2-8', data.BG_TOGGLE);
    updateToggles('cb3-8', data.REVERSE_TOGGLE);

}

// DropdownList script for LED Modes
const selectedItemModes = document.querySelector('#selected-item-modes');

selectedItemModes.addEventListener('change', () => {
    const selectedModeId = selectedItemModes.value;
    console.log('Selected Mode ID:', selectedModeId); // Debugging statement

    // Send a WebSocket message for changing the LED mode
    sendData('ChangeLEDModeAction', { mode: selectedModeId });
    sendData('RequestValues');
});

// DropdownList script for Animations
const selectedItemAnimations = document.querySelector('#selected-item-animations');

selectedItemAnimations.addEventListener('change', () => {
    const selectedAnimationId = selectedItemAnimations.value;
    console.log('Selected Animation ID:', selectedAnimationId); // Debugging statement

    // Send a WebSocket message for changing the animation
    sendData('ChangeAnimationAction', { animation: selectedAnimationId });
});


// DropdownList script for Colors
const selectedItemColors = document.querySelector('#selected-item-colors');

selectedItemColors.addEventListener('change', () => {
    const selectedColorsId = selectedItemColors.value;
    console.log('Selected Color ID:', selectedColorsId); // Debugging statement

    // Send a WebSocket message for changing the animation
    sendData('ChangeColorAction', { color: selectedColorsId });
    sendData('RequestValues');
});

// Hue Slider Code
const thumb = document.querySelector('.thumb');
const track = document.querySelector('.track');
const hueValueInput = document.getElementById('HUE');
let isDragging = false;

// Function to handle mouse and touch move
function handleMove(xPosition) {
    const maxPosition = track.offsetWidth - thumb.offsetWidth;
    const minPosition = 0;

    // Ensure the thumb stays within the track bounds
    xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

    thumb.style.left = xPosition + 'px';

    // Calculate the hue value based on thumb position
    const hue = (xPosition / maxPosition) * 360;

    // Map the hue value to the 0-255 range for the slider
    const mappedHue = Math.round((hue / 360) * 255);
    hueValueInput.value = mappedHue; // Store the mapped hue value in the hidden input

    // Set the thumb's background color based on the hue value
    thumb.style.backgroundColor = `hsl(${hue}, 100%, 50%)`;

    // Trigger the input event manually on the hue slider
    const inputEvent = new Event('input', {
        bubbles: true,
        cancelable: true,
    });
    hueValueInput.dispatchEvent(inputEvent);
}

// Mouse drag
thumb.addEventListener('mousedown', (e) => {
    isDragging = true;
    thumb.style.transition = 'none';
    const offsetX = e.clientX - thumb.getBoundingClientRect().left;

    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    function onMouseMove(e) {
        if (!isDragging) return;

        const newPosition = e.clientX - track.getBoundingClientRect().left - offsetX;
        handleMove(newPosition);
    }

    function onMouseUp() {
        isDragging = false;
        thumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
    }
});

// Touch event handling for mobile devices
thumb.addEventListener('touchstart', (e) => {
    isDragging = true;
    thumb.style.transition = 'none';
    const offsetX = e.touches[0].clientX - thumb.getBoundingClientRect().left;

    document.addEventListener('touchmove', onTouchMove);
    document.addEventListener('touchend', onTouchEnd);

    function onTouchMove(e) {
        if (!isDragging) return;

        const newPosition = e.touches[0].clientX - track.getBoundingClientRect().left - offsetX;
        handleMove(newPosition);
    }

    function onTouchEnd() {
        isDragging = false;
        thumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('touchmove', onTouchMove);
        document.removeEventListener('touchend', onTouchEnd);
    }
});

// Track click
track.addEventListener('click', (e) => {
    const clickX = e.clientX - track.getBoundingClientRect().left;
    const thumbPosition = clickX - thumb.offsetWidth / 2;
    handleMove(thumbPosition);

    // Restore smooth transition for the thumb
    thumb.style.transition = 'left 0.3s ease';
});



// Brightness Slider Code
const brightnessThumb = document.querySelector('.brightness-slider .thumb');
const brightnessTrack = document.querySelector('.brightness-slider .track');
const brightnessValueInput = document.getElementById('BRIGHTNESS');
let isBrightnessDragging = false;

// Function to update the brightness slider's track gradient based on the hue value
function updateBrightnessTrackGradient(hue) {
    const track = document.querySelector('.brightness-slider .track');
    // Map the hue value from 0-360 to 0-255
    const mappedHue = Math.round((hue / 180) * 255);
    // Update the track gradient CSS with the mapped hue value
    track.style.background = `linear-gradient(to right, #000, hsl(${mappedHue}, 100%, 50%))`;
}

// Function to handle brightness changes
function handleBrightnessMove(xPosition) {
    const maxPosition = brightnessTrack.offsetWidth - brightnessThumb.offsetWidth;
    const minPosition = 0;

    // Ensure the thumb stays within the track bounds
    xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

    brightnessThumb.style.left = xPosition + 'px';

    // Calculate the brightness value based on thumb position
    const brightness = Math.round((xPosition / maxPosition) * maxBrightness);

    // Update the brightness value in the hidden input
    brightnessValueInput.value = brightness;
    // Trigger the input event manually on the hue slider
    const inputEvent = new Event('input', {
        bubbles: true,
        cancelable: true,
    });
    brightnessValueInput.dispatchEvent(inputEvent);
}

// Update the brightness slider's track gradient when the hue slider value changes
hueValueInput.addEventListener('input', () => {
    const hue = parseInt(hueValueInput.value);
    updateBrightnessTrackGradient(hue);
});

brightnessThumb.addEventListener('mousedown', (e) => {
    isBrightnessDragging = true;
    brightnessThumb.style.transition = 'none';
    const offsetX = e.clientX - brightnessThumb.getBoundingClientRect().left;

    document.addEventListener('mousemove', onBrightnessMouseMove);
    document.addEventListener('mouseup', onBrightnessMouseUp);

    function onBrightnessMouseMove(e) {
        if (!isBrightnessDragging) return;

        const newPosition = e.clientX - brightnessTrack.getBoundingClientRect().left - offsetX;
        handleBrightnessMove(newPosition);
    }

    function onBrightnessMouseUp() {
        isBrightnessDragging = false;
        brightnessThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('mousemove', onBrightnessMouseMove);
        document.removeEventListener('mouseup', onBrightnessMouseUp);
    }
});

brightnessThumb.addEventListener('touchstart', (e) => {
    isBrightnessDragging = true;
    brightnessThumb.style.transition = 'none';
    const offsetX = e.touches[0].clientX - brightnessThumb.getBoundingClientRect().left;

    document.addEventListener('touchmove', onBrightnessTouchMove);
    document.addEventListener('touchend', onBrightnessTouchEnd);

    function onBrightnessTouchMove(e) {
        if (!isBrightnessDragging) return;

        const newPosition = e.touches[0].clientX - brightnessTrack.getBoundingClientRect().left - offsetX;
        handleBrightnessMove(newPosition);
    }

    function onBrightnessTouchEnd() {
        isBrightnessDragging = false;
        brightnessThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('touchmove', onBrightnessTouchMove);
        document.removeEventListener('touchend', onBrightnessTouchEnd);
    }
});

brightnessTrack.addEventListener('click', (e) => {
    const clickX = e.clientX - brightnessTrack.getBoundingClientRect().left;
    const thumbPosition = clickX - brightnessThumb.offsetWidth / 2;
    handleBrightnessMove(thumbPosition);

    // Restore smooth transition for the brightness thumb
    brightnessThumb.style.transition = 'left 0.3s ease';
});

// Initial setup for the brightness slider
const initialBrightness = parseInt(brightnessValueInput.value);
const maxBrightness = 255; // Maximum brightness value
const initialBrightnessPosition = (initialBrightness / maxBrightness) * (brightnessTrack.offsetWidth - brightnessThumb.offsetWidth);
const hue = parseInt(hueValueInput.value);
brightnessThumb.style.left = initialBrightnessPosition + 'px';
// Initialize the brightness slider's track gradient based on the initial hue
updateBrightnessTrackGradient(hue);


// Fade slider
const fadeThumb = document.querySelector('.fade-slider .thumb');
const fadeTrack = document.querySelector('.fade-slider .track');
const fadeValueInput = document.getElementById('FADE');
let isFadeDragging = false;

// Function to handle mouse and touch move
function handleFadeMove(xPosition) {
    const maxPosition = fadeTrack.offsetWidth - fadeThumb.offsetWidth;
    const minPosition = 0;

    // Ensure the thumb stays within the track bounds
    xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

    fadeThumb.style.left = xPosition + 'px';

    // Calculate the fade value based on thumb position
    const fade = Math.round((xPosition / maxPosition) * 255);

    fadeValueInput.value = fade;

    // Trigger the input event manually on the fade slider
    const inputEvent = new Event('input', {
        bubbles: true,
        cancelable: true,
    });
    fadeValueInput.dispatchEvent(inputEvent);
}

// Mouse drag
fadeThumb.addEventListener('mousedown', (e) => {
    isFadeDragging = true;
    fadeThumb.style.transition = 'none';
    const offsetX = e.clientX - fadeThumb.getBoundingClientRect().left;

    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    function onMouseMove(e) {
        if (!isFadeDragging) return;

        const newPosition = e.clientX - fadeTrack.getBoundingClientRect().left - offsetX;
        handleFadeMove(newPosition);
    }

    function onMouseUp() {
        isFadeDragging = false;
        fadeThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
    }
});

// Touch event handling for mobile devices
fadeThumb.addEventListener('touchstart', (e) => {
    isFadeDragging = true;
    fadeThumb.style.transition = 'none';
    const offsetX = e.touches[0].clientX - fadeThumb.getBoundingClientRect().left;

    document.addEventListener('touchmove', onTouchMove);
    document.addEventListener('touchend', onTouchEnd);

    function onTouchMove(e) {
        if (!isFadeDragging) return;

        const newPosition = e.touches[0].clientX - fadeTrack.getBoundingClientRect().left - offsetX;
        handleFadeMove(newPosition);
    }

    function onTouchEnd() {
        isFadeDragging = false;
        fadeThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('touchmove', onTouchMove);
        document.removeEventListener('touchend', onTouchEnd);
    }
});

// Track click
fadeTrack.addEventListener('click', (e) => {
    const clickX = e.clientX - fadeTrack.getBoundingClientRect().left;
    const thumbPosition = clickX - fadeThumb.offsetWidth / 2;
    handleFadeMove(thumbPosition);

    // Restore smooth transition for the thumb
    fadeThumb.style.transition = 'left 0.3s ease';
});



// Splash slider
const splashThumb = document.querySelector('.splash-slider .thumb');
const splashTrack = document.querySelector('.splash-slider .track');
const splashValueInput = document.getElementById('SPLASH');
let isSplashDragging = false;

// Function to handle mouse and touch move
function handleSplashMove(xPosition) {
    const maxPosition = splashTrack.offsetWidth - splashThumb.offsetWidth;
    const minPosition = 0;

    // Ensure the thumb stays within the track bounds
    xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

    splashThumb.style.left = xPosition + 'px';

    // Calculate the splash value based on thumb position
    const splash = Math.round((xPosition / maxPosition) * 16);

    splashValueInput.value = splash;

    // Trigger the input event manually on the splash slider
    const inputEvent = new Event('input', {
        bubbles: true,
        cancelable: true,
    });
    splashValueInput.dispatchEvent(inputEvent);
}

// Mouse drag
splashThumb.addEventListener('mousedown', (e) => {
    isSplashDragging = true;
    splashThumb.style.transition = 'none';
    const offsetX = e.clientX - splashThumb.getBoundingClientRect().left;

    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    function onMouseMove(e) {
        if (!isSplashDragging) return;

        const newPosition = e.clientX - splashTrack.getBoundingClientRect().left - offsetX;
        handleSplashMove(newPosition);
    }

    function onMouseUp() {
        isSplashDragging = false;
        splashThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
    }
});

// Touch event handling for mobile devices
splashThumb.addEventListener('touchstart', (e) => {
    isSplashDragging = true;
    splashThumb.style.transition = 'none';
    const offsetX = e.touches[0].clientX - splashThumb.getBoundingClientRect().left;

    document.addEventListener('touchmove', onTouchMove);
    document.addEventListener('touchend', onTouchEnd);

    function onTouchMove(e) {
        if (!isSplashDragging) return;

        const newPosition = e.touches[0].clientX - splashTrack.getBoundingClientRect().left - offsetX;
        handleSplashMove(newPosition);
    }

    function onTouchEnd() {
        isSplashDragging = false;
        splashThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('touchmove', onTouchMove);
        document.removeEventListener('touchend', onTouchEnd);
    }
});

// Track click
splashTrack.addEventListener('click', (e) => {
    const clickX = e.clientX - splashTrack.getBoundingClientRect().left;
    const thumbPosition = clickX - splashThumb.offsetWidth / 2;
    handleSplashMove(thumbPosition);

    // Restore smooth transition for the thumb
    splashThumb.style.transition = 'left 0.3s ease';
});

const initialSplash = parseInt(splashValueInput.value);
const maxSplash = 16;
const initialSplashPosition = (initialSplash / maxSplash) * (splashTrack.offsetWidth - splashThumb.offsetWidth);
splashThumb.style.left = initialSplashPosition + 'px';



// Background slider
const bgThumb = document.querySelector('.bg-slider .thumb');
const bgTrack = document.querySelector('.bg-slider .track');
const bgValueInput = document.getElementById('BG');
let isBgDragging = false;

// Function to handle mouse and touch move for background slider
function handleBgMove(xPosition) {
    const maxPosition = bgTrack.offsetWidth - bgThumb.offsetWidth;
    const minPosition = 0;

    // Ensure the thumb stays within the track bounds
    xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

    bgThumb.style.left = xPosition + 'px';

    // Calculate the background brightness value based on thumb position
    const brightness = Math.round((xPosition / maxPosition) * 255);

    bgValueInput.value = brightness;

    // Trigger the input event manually on the background slider
    const inputEvent = new Event('input', {
        bubbles: true,
        cancelable: true,
    });
    bgValueInput.dispatchEvent(inputEvent);
}

// Mouse drag for background slider
bgThumb.addEventListener('mousedown', (e) => {
    isBgDragging = true;
    bgThumb.style.transition = 'none';
    const offsetX = e.clientX - bgThumb.getBoundingClientRect().left;

    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);

    function onMouseMove(e) {
        if (!isBgDragging) return;

        const newPosition = e.clientX - bgTrack.getBoundingClientRect().left - offsetX;
        handleBgMove(newPosition);
    }

    function onMouseUp() {
        isBgDragging = false;
        bgThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('mousemove', onMouseMove);
        document.removeEventListener('mouseup', onMouseUp);
    }
});

// Touch event handling for mobile devices for background slider
bgThumb.addEventListener('touchstart', (e) => {
    isBgDragging = true;
    bgThumb.style.transition = 'none';
    const offsetX = e.touches[0].clientX - bgThumb.getBoundingClientRect().left;

    document.addEventListener('touchmove', onTouchMove);
    document.addEventListener('touchend', onTouchEnd);

    function onTouchMove(e) {
        if (!isBgDragging) return;

        const newPosition = e.touches[0].clientX - bgTrack.getBoundingClientRect().left - offsetX;
        handleBgMove(newPosition);
    }

    function onTouchEnd() {
        isBgDragging = false;
        bgThumb.style.transition = 'left 0.3s ease'; // Restore smooth transition
        document.removeEventListener('touchmove', onTouchMove);
        document.removeEventListener('touchend', onTouchEnd);
    }
});

// Track click for background slider
bgTrack.addEventListener('click', (e) => {
    const clickX = e.clientX - bgTrack.getBoundingClientRect().left;
    const thumbPosition = clickX - bgThumb.offsetWidth / 2;
    handleBgMove(thumbPosition);

    // Restore smooth transition for the thumb
    bgThumb.style.transition = 'left 0.3s ease';
});

// Set the initial background brightness
const initialBgBrightness = parseInt(bgValueInput.value);

// Calculate the initial thumb position based on the initial value
const initialBGBrightnessThumbPosition = (initialBgBrightness / 255) * (bgTrack.offsetWidth - bgThumb.offsetWidth);
bgThumb.style.left = initialBGBrightnessThumbPosition + 'px';

function handleWindowResize() {
    const maxHuePosition = track.offsetWidth - thumb.offsetWidth;
    const maxBrightnessPosition =  brightnessTrack.offsetWidth - brightnessThumb.offsetWidth;
    const maxFadePosition = fadeTrack.offsetWidth - fadeThumb.offsetWidth;
    const maxSplashPosition = splashTrack.offsetWidth - splashThumb.offsetWidth;
    const maxBGPosition = bgTrack.offsetWidth - bgThumb.offsetWidth;

    const mappedHue = (parseInt(hueValueInput.value) / 255) * 360;
    const mappedBrightness = (parseInt(brightnessValueInput.value));
    const mappedFade = (parseInt(fadeValueInput.value));
    const mappedSplash = (parseInt(splashValueInput.value));
    const mappedBG = (parseInt(bgValueInput.value)); 

    // Calculate the new position based on the mapped hue value
    const newPosition = (mappedHue / 360) * maxHuePosition;
    const newBrightnessPosition = (mappedBrightness / 255) * maxBrightnessPosition;
    const newFadePosition = (mappedFade / 255) * maxFadePosition;
    const newSplashPosition = (mappedSplash / 16) * maxSplashPosition;
    const newBGPosition = (mappedBG / 255) * maxBGPosition;

    // Update the thumb's position
    thumb.style.left = newPosition + 'px';
    brightnessThumb.style.left = newBrightnessPosition + 'px';
    fadeThumb.style.left = newFadePosition + 'px';
    splashThumb.style.left = newSplashPosition + 'px';
    bgThumb.style.left = newBGPosition + 'px';

    // Calculate the background color based on the hue value
    const backgroundColor = `hsl(${mappedHue}, 100%, 50%)`;

    // Update the thumb's background color
    thumb.style.backgroundColor = backgroundColor;
    thumb.style.transition = 'left 0s ease';
    brightnessThumb.style.transition = 'left 0s ease';
    fadeThumb.style.transition = 'left 0s ease';
    splashThumb.style.transition = 'left 0s ease';
    bgThumb.style.transition = 'left 0s ease';
}
// Add the event listener for window resize
window.addEventListener('resize', handleWindowResize);


function createButtonListener(button, values, index, actionName) {
    button.addEventListener("click", function () {
        index = (index + 1) % values.length;
        button.textContent = values[index];
        sendData(actionName, { value: index });
        console.log('Sending:', actionName + index); // Debugging
    });
}

const sizes = ["88 Key", "76 Key", "73 Key", "61 Key", "49 Key"];
let sizeIndex = 0;
const pianoSizeButton = document.getElementById("PianoSize");
createButtonListener(pianoSizeButton, sizes, sizeIndex, 'PianoSizeAction');

const ratios = ["1:2", "1:1"];
let ratioIndex = 0;
const ledScaleRatioButton = document.getElementById("LEDScaleRatio");
createButtonListener(ledScaleRatioButton, ratios, ratioIndex, 'LedScaleRatioAction');

function createCheckboxListener(checkbox, actionName) {
    checkbox.addEventListener('change', function () {
        const value = this.checked ? 1 : 0;
        sendData(actionName, { value });
        console.log('Sending:', actionName + value); // Debugging
    });
}

const cb2Checkbox1 = document.getElementById('cb1-8');
const cb2Checkbox2 = document.getElementById('cb2-8');
const cb2Checkbox3 = document.getElementById('cb3-8');
const cb2Checkbox4 = document.getElementById('cb4-8');

createCheckboxListener(cb2Checkbox1, 'FixAction');
createCheckboxListener(cb2Checkbox2, 'BGAction');
createCheckboxListener(cb2Checkbox3, 'DirectionAction');
createCheckboxListener(cb2Checkbox4, 'HueChangeAction');


const fileLink = document.getElementById('fileLink');
const confirmationDialog = document.getElementById('confirmationDialog');
const DownloadOTAUpdatesButton = document.getElementById('Update');
const UploadOTAButton = document.getElementById('Download');
const CancelButton = document.getElementById('Cancel');
const boardSelect = document.getElementById('boardSelect');
const binarySelect = document.getElementById('binarySelect');

let isDialogOpen = false;

// Add event listeners for the "Local," "Online," and "Cancel" buttons outside the fileLink event listener.
DownloadOTAUpdatesButton.addEventListener('click', () => {
    // If the custom "Local" button is clicked, proceed with the code for local upload.
    confirmationDialog.style.display = 'none';
    isDialogOpen = false;

    doOTA();
});

UploadOTAButton.addEventListener('click', () => {
    // If the custom "Online" button is clicked, fetch and handle the binary file.
    const selectedBoard = boardSelect.value;
    const selectedBinary = binarySelect.value;

    if (selectedBoard && selectedBinary) {
        // Now you have both the selected board and binary type.
        // You can call the fetchAssetsList function with these values.
        fetchAssetsList(selectedBoard, selectedBinary);
    } else {
        console.error('Please select both board and binary type.');
    }

    confirmationDialog.style.display = 'none';
    isDialogOpen = false;
});


CancelButton.addEventListener('click', () => {
    // If the custom "Cancel" button is clicked, close the dialog without proceeding.
    confirmationDialog.style.display = 'none';
    isDialogOpen = false;
});

fileLink.addEventListener('click', () => {
    if (!isDialogOpen) {
        // Display the custom dialog.
        confirmationDialog.style.display = 'block';
        isDialogOpen = true;
    }
});

function downloadFile(url, fileName) {
    // Create an anchor element for triggering the download
    const anchor = document.createElement('a');
    anchor.href = url;
    anchor.download = fileName;

    // Trigger a click event on the anchor to start the download
    anchor.click();
    alert('Delete downloaded files after you upload them to ESP32 board!');
}

function fetchAssetsList(board, fileType) {
    const githubApiUrl = `https://api.github.com/repos/serifpersia/pianoled-esp32/releases/latest`;

    fetch(githubApiUrl)
        .then(response => response.json())
        .then(data => {
        const assets = data.assets;

        if (assets.length > 0) {
            console.log('Available assets in the latest release:');
            assets.forEach(asset => {
                console.log(asset.name);
            });

            // Construct the binary file name based on the board and fileType
            const binaryFileName = `${board}_${fileType}.bin`;
            const binaryFile = assets.find(asset => asset.name === binaryFileName);

            if (binaryFile) {
                console.log(`Fetched binary file: ${binaryFile.name}`);

                // Use the downloadFile function to download the binary file
                downloadFile(binaryFile.browser_download_url, binaryFile.name);
            } else {
                console.error(`Binary file ${binaryFileName} not found.`);
            }
        } else {
            console.error('No assets found in the latest release.');
        }
    })
        .catch(error => {
        console.error('Error fetching release assets:', error);
        alert('No files found on Github Repository!');
    });
}


function doOTA() {
    window.location.href = '/update'; // This will change the URL to '/update'

}

const maxCurrentInput = document.getElementById("maxCurrent");
let typingTimer;
const typingTimeout = 2000; // Adjust the timeout value as needed
let alertShown = false;
let stripLEDalertShown = false;

// Function to display a warning when the input field is focused
maxCurrentInput.addEventListener("focus", function() {
    if (!alertShown) {
        alert("Make sure your power supply can handle the current you enter.");
        alertShown = true;
    }
});

maxCurrentInput.addEventListener("input", function() {
    clearTimeout(typingTimer);
    typingTimer = setTimeout(function() {
        const minValue = parseFloat(maxCurrentInput.min);
        const maxValue = parseFloat(maxCurrentInput.max);
        let enteredValue = parseFloat(maxCurrentInput.value);

        if (isNaN(enteredValue)) {
            // If the entered value is not a number, set it to the minimum value
            maxCurrentInput.value = minValue;
            enteredValue = minValue;
        } else {
            // Ensure the entered value is within the specified range
            enteredValue = Math.min(maxValue, Math.max(minValue, enteredValue));
            maxCurrentInput.value = enteredValue;
        }

        // Send the entered value to the socket
        sendData('CurrentAction', { value: enteredValue });
        console.log('Sending:', 'CurrentAction', enteredValue);
    }, typingTimeout);
});

const ledDataPinInput = document.getElementById("ledDataPin");


// Function to display a warning when the input field is focused
ledDataPinInput.addEventListener("focus", function() {
    if (!stripLEDalertShown) {
        alert("Enter valid GPIO pin number on your ESP32 S2/S3 you want to use for W2812 LED Strip Data");
        stripLEDalertShown = true;
    }
});

ledDataPinInput.addEventListener("input", function() {
    clearTimeout(typingTimer);
    typingTimer = setTimeout(function() {
        const minValue = parseFloat(ledDataPinInput.min);
        const maxValue = parseFloat(ledDataPinInput.max);
        let enteredValue = parseFloat(ledDataPinInput.value);

        if (isNaN(enteredValue)) {
            // If the entered value is not a number, set it to the minimum value
            ledDataPinInput.value = minValue;
            enteredValue = minValue;
        } else {
            // Ensure the entered value is within the specified range
            enteredValue = Math.min(maxValue, Math.max(minValue, enteredValue));
            ledDataPinInput.value = enteredValue;
        }

        alert("ESP will now restart. Reload the webpage after few seconds!");
        // Send the action and entered value to the socket
        sendData('LedDataPinAction', { value: enteredValue });
        console.log('Sending:', 'LedDataPinAction', enteredValue);
    }, typingTimeout);
});

// Call the init function when the window loads
window.onload = function (event) {
    init();
};

