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

// Event listeners for button and sliders
//document.getElementById('BTN_COLOR').addEventListener('click', function () {
//   sendData('ChangeColor');
//  console.log('ChangeColor');
//});

document.getElementById('HUE').addEventListener('input', function () {
    var value = parseInt(document.getElementById('HUE').value);
    sendData('Hue', { value: value });
    console.log(value);
});

document.getElementById('BRIGHTNESS').addEventListener('input', function () {
    var value = parseInt(document.getElementById('BRIGHTNESS').value);
    sendData('Brightness', { value: value });
    console.log(value);
});

document.getElementById('FADE').addEventListener('input', function () {
    var value = parseInt(document.getElementById('FADE').value);
    var invertedValue = 255 - value;
    sendData('Fade', { value: invertedValue });
    console.log(invertedValue);
});

document.getElementById('SPLASH').addEventListener('input', function () {
    var value = parseInt(document.getElementById('SPLASH').value);
    sendData('Splash', { value: value });
    console.log(value);
});

document.getElementById('BG').addEventListener('input', function () {
    var value = parseInt(document.getElementById('BG').value);
    sendData('Background', { value: value });
    console.log(value);
});


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


// DropdownList script for LED Modes
const selectedItemModes = document.querySelector('#selected-item-modes');
const dropdownListModes = document.querySelector('#dropdown-list-modes');

// Initially hide the dropdown list for LED Modes
dropdownListModes.style.display = 'none';

// Add click event listeners to each dropdown item for LED Modes
const dropdownItemsModes = document.querySelectorAll('.dropdown-list-modes .dropdown-item');
dropdownItemsModes.forEach((item) => {
    item.addEventListener('click', () => {
        console.log('Item clicked:', item.textContent); // Debugging statement
        const selectedModeId = parseInt(item.getAttribute('data-mode-id'), 10);
        console.log('Selected Mode ID:', selectedModeId); // Debugging statement
        selectedItemModes.textContent = item.textContent;
        dropdownListModes.style.display = 'none'; // Compact the dropdown list for LED Modes
        console.log('Sending:', 'ChangeLEDModeAction' + selectedModeId); // Debugging statement

        // Send a WebSocket message for changing the LED mode
        sendData('ChangeLEDModeAction', { mode: selectedModeId });
    });
});

selectedItemModes.addEventListener('click', () => {
    console.log('Dropdown clicked'); // Debugging statement
    // Toggle the visibility of the dropdown list for LED Modes
    if (dropdownListModes.style.display === 'none' || dropdownListModes.style.display === '') {
        console.log('Opening dropdown'); // Debugging statement
        dropdownListModes.style.display = 'block';
    } else {
        console.log('Closing dropdown'); // Debugging statement
        dropdownListModes.style.display = 'none';
    }
});

// DropdownList script for Animations
const selectedItemAnimations = document.querySelector('#selected-item-animations');
const dropdownListAnimations = document.querySelector('#dropdown-list-animations');

// Initially hide the dropdown list for Animations
dropdownListAnimations.style.display = 'none';

// Add click event listeners to each dropdown item for Animations
const dropdownItemsAnimations = document.querySelectorAll('.dropdown-list-animations .dropdown-item');
dropdownItemsAnimations.forEach((item) => {
    item.addEventListener('click', () => {
        console.log('Item clicked:', item.textContent); // Debugging statement
        const selectedAnimationId = parseInt(item.getAttribute('data-animation-id'), 10);
        console.log('Selected Animation ID:', selectedAnimationId); // Debugging statement
        selectedItemAnimations.textContent = item.textContent;
        dropdownListAnimations.style.display = 'none'; // Compact the dropdown list for Animations
        console.log('Sending:', 'ChangeAnimationAction' + selectedAnimationId); // Debugging statement

        // Send a WebSocket message for changing the animation
        sendData('ChangeAnimationAction', { animation: selectedAnimationId });
    });
});

selectedItemAnimations.addEventListener('click', () => {
    console.log('Dropdown clicked'); // Debugging statement
    // Toggle the visibility of the dropdown list for Animations
    if (dropdownListAnimations.style.display === 'none' || dropdownListAnimations.style.display === '') {
        console.log('Opening dropdown'); // Debugging statement
        dropdownListAnimations.style.display = 'block';
    } else {
        console.log('Closing dropdown'); // Debugging statement
        dropdownListAnimations.style.display = 'none';
    }
});


// Call the init function when the window loads
window.onload = function (event) {
    init();
};


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


const sizes = ["88", "76", "73", "61", "49"];
let sizeIndex = 0;

const ratios = ["1:2", "1:1"];
let ratioIndex = 0;

const pianoSizeButton = document.getElementById("PianoSize");
const ledScaleRatioButton = document.getElementById("LEDScaleRatio");

pianoSizeButton.addEventListener("click", function () {
    sizeIndex = (sizeIndex + 1) % sizes.length;
    pianoSizeButton.textContent = sizes[sizeIndex] + " Key";
    sendData('PianoSizeAction', { value: sizeIndex });
    console.log('Sending:', 'PianoSizeAction' + sizeIndex); // Debugging 
});

ledScaleRatioButton.addEventListener("click", function () {
    ratioIndex = (ratioIndex + 1) % ratios.length;
    ledScaleRatioButton.textContent = ratios[ratioIndex];
    sendData('LedScaleRatioAction', { value: ratioIndex });
    console.log('Sending:', 'LedScaleRatioAction' + ratioIndex); // Debugging 
});

const cb2Checkbox = document.getElementById('cb2-8');

cb2Checkbox.addEventListener('change', function() {
    // Check if it's toggle id 2
    if (this.checked) {
        // Send data to the socket with action BGAction and value 1
        sendData('BGAction', { value: 1 });
        console.log('Sending:', 'BGAction' + 1); // Debugging 
    } else {
        // Send data to the socket with action BGAction and value 0
        sendData('BGAction', { value: 0 });
        console.log('Sending:', 'BGAction' + 0); // Debugging 
    }
});

const fileLink = document.getElementById('fileLink');
const confirmationDialog = document.getElementById('confirmationDialog');
const DownloadOTAUpdatesButton = document.getElementById('Update');
const UploadOTAButton = document.getElementById('Download');
const CancelButton = document.getElementById('Cancel');
const binarySelect = document.getElementById('binarySelect');

let isDialogOpen = false;


DownloadOTAUpdatesButton.addEventListener('click', () => {
    // If the custom "Local" button is clicked, proceed with the code for local upload.
    confirmationDialog.style.display = 'none';
    isDialogOpen = false;

    doOTA();
});

UploadOTAButton.addEventListener('click', () => {
    // If the custom "Online" button is clicked, fetch and handle the binary file.
    const selectedBinary = binarySelect.value;

    if (selectedBinary) {
        // Now you have the selected binary type.
        // You can call the fetchAssetsList function with this value.
        fetchAssetsList(selectedBinary);
    } else {
        console.error('Please select a binary type.');
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
}

function fetchAssetsList(fileType) {
    const githubApiUrl = `https://api.github.com/repos/serifpersia/pianoled-esp32/releases/latest`;

    fetch(githubApiUrl)
        .then(response => {
            if (!response.ok) {
                throw new Error(`Failed to fetch data from GitHub API: ${response.status} ${response.statusText}`);
            }
            return response.json();
        })
        .then(data => {
            const assets = data.assets;

            if (assets.length > 0) {
                console.log('Available assets in the latest release:');
                assets.forEach(asset => {
                    console.log(asset.name);
                });

                // Construct the binary file name based on the fileType
                const binaryFileName = `${fileType}.bin`;
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
            // Display an alert if there was an error fetching data
            alert('No files found on Github Repository!');
        });
}


function doOTA() {
    window.location.href = '/update'; // This will change the URL to '/update'

}