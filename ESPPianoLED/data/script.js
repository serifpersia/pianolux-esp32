var Socket;
document.getElementById('BTN_COLOR').addEventListener('click', button_changeColor);
document.getElementById('HUE').addEventListener('input', slider1_changeValue);
document.getElementById('BRIGHTNESS').addEventListener('input', slider2_changeValue);
document.getElementById('FADE').addEventListener('input', slider3_changeValue);
function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
}

function button_changeColor() {
    Socket.send('ChangeColor');
    console.log('ChangeColor');
}


function slider1_changeValue() {
    var value = document.getElementById('HUE').value;
    Socket.send('SliderAction1:' + value);
    console.log(value);
}


function slider2_changeValue() {
    var value = document.getElementById('BRIGHTNESS').value;
    Socket.send('SliderAction2:' + value);
    console.log(value);
}


function slider3_changeValue() {
    var value = document.getElementById('FADE').value;
    var invertedValue = 255 - value; // Calculate the inverted value
    Socket.send('SliderAction3:' + invertedValue); // Send the inverted value
    console.log(invertedValue);
}



window.onload = function(event) {
    init();
}

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
    Socket.send('ChangeLEDModeAction:' + selectedModeId);
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

selectedItemAnimations.addEventListener('click', () => {
    // Toggle the visibility of the dropdown list for Animations
    if (dropdownListAnimations.style.display === 'none' || dropdownListAnimations.style.display === '') {
        dropdownListAnimations.style.display = 'block';
    } else {
        dropdownListAnimations.style.display = 'none';
    }
});

// Add click event listeners to each dropdown item for Animations
const dropdownItemsAnimations = document.querySelectorAll('.dropdown-list-animations .dropdown-item');
dropdownItemsAnimations.forEach((item) => {
    item.addEventListener('click', () => {
        selectedItemAnimations.textContent = item.textContent;
        dropdownListAnimations.style.display = 'none'; // Compact the dropdown list for Animations
    });
});



// Hue Slider Code
const thumb = document.querySelector('.thumb');
const track = document.querySelector('.track');
const hueValueInput = document.getElementById('HUE');
let isDragging = false;

// Function to handle mouse and touch move
function handleMove(xPosition) {
    const maxPosition = track.offsetWidth - thumb.offsetWidth;

    if (xPosition >= 0 && xPosition <= maxPosition) {
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
}




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

track.addEventListener('click', (e) => {
    const clickX = e.clientX - track.getBoundingClientRect().left;
    const thumbPosition = clickX - thumb.offsetWidth / 2;
    handleMove(thumbPosition);

    // Restore smooth transition for the thumb
    thumb.style.transition = 'left 0.3s ease';
});

// Initial setup to set the thumb's color based on the initial hue value
const initialHue = parseInt(hueValueInput.value); // Get the initial hue value from the hidden input
const initialThumbPosition = (initialHue / 360) * (track.offsetWidth - thumb.offsetWidth);
thumb.style.left = initialThumbPosition + 'px';
thumb.style.backgroundColor = `hsl(${initialHue}, 100%, 50%)`;





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

    if (xPosition >= 0 && xPosition <= maxPosition) {
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
// Set the initial background color for the thumb




//Fade slider
const fadeThumb = document.querySelector('.fade-slider .thumb');
const fadeTrack = document.querySelector('.fade-slider .track');
const fadeValueInput = document.getElementById('FADE');
let isFadeDragging = false;

// Function to handle mouse and touch move
function handleFadeMove(xPosition) {
    const maxPosition = fadeTrack.offsetWidth - fadeThumb.offsetWidth;

    if (xPosition >= 0 && xPosition <= maxPosition) {
        fadeThumb.style.left = xPosition + 'px';

        // Calculate the hue value based on thumb position
        const fade =  Math.round((xPosition / maxPosition) * 255);

        fadeValueInput.value = fade;

        // Trigger the input event manually on the hue slider
        const inputEvent = new Event('input', {
            bubbles: true,
            cancelable: true,
        });
        fadeValueInput.dispatchEvent(inputEvent);
    }
}
//Mouse drag
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

fadeTrack.addEventListener('click', (e) => {
    const clickX = e.clientX - fadeTrack.getBoundingClientRect().left;
    const thumbPosition = clickX - fadeThumb.offsetWidth / 2;
    handleFadeMove(thumbPosition);

    // Restore smooth transition for the thumb
    fadeThumb.style.transition = 'left 0.3s ease';
});

