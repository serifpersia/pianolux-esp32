var Socket;

// Function to initialize WebSocket
function init() {
  Socket = new WebSocket("ws://" + window.location.hostname + ":81/");

  // Add error event handler
  Socket.addEventListener("error", function(error) {
    console.error("WebSocket error:", error);
    // Handle the error here, e.g., display an error message to the user.
  });

  // Add close event handler
  Socket.addEventListener("close", function(event) {
    if (event.wasClean) {
      console.log(
        "WebSocket closed cleanly, code=" +
          event.code +
          ", reason=" +
          event.reason
      );
    } else {
      console.error("WebSocket connection died");
      // You may want to attempt to reconnect here.
    }
  });

  // Add open event handler
  Socket.addEventListener("open", function(event) {
    console.log("WebSocket connection opened");
    sendData("RequestValues");
  });

  // Event listener to handle updates from the server
  Socket.addEventListener("message", function(event) {
    var data = JSON.parse(event.data);
    console.log("Received data from the server:", data);

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
      console.error("Error sending data:", error);
      // Handle the error here, e.g., display an error message to the user.
    }
  } else {
    console.error("WebSocket connection not open");
    // Handle the error here, e.g., display an error message to the user or attempt to reconnect.
  }
}

function addInputListener(controlId, dataKey, factor) {
  document.getElementById(controlId).addEventListener("input", function() {
    const value = parseInt(this.value);
    sendData(dataKey, { value: value });
    console.log(dataKey);
    console.log(value);
  });
}

addInputListener("HUE", "Hue", 1);
addInputListener("SATURATION", "Saturation", 1);
addInputListener("BRIGHTNESS", "Brightness", 1);
addInputListener("FADE", "Fade", 1);
addInputListener("SPLASH", "Splash", 1);
addInputListener("BG", "Background", 1);
addInputListener("SPLIT", "Split", 1);

var navbar = document.querySelector(".navbar");
var scrolling = false;

function handleScroll() {
  if (!scrolling) {
    navbar.style.opacity = "1"; // Show the navbar
    navbar.style.pointerEvents = "auto"; // Enable pointer events
  }

  scrolling = true;

  // Use a timeout to hide the navbar after a delay when scrolling stops
  clearTimeout(window.scrollTimeout);
  window.scrollTimeout = setTimeout(function() {
    scrolling = false;
    navbar.style.opacity = "0"; // Hide the navbar
    navbar.style.pointerEvents = "none"; // Disable pointer events
  }, 1500); // Adjust the delay time (in milliseconds) as needed
}

// Add an event listener for the scroll event
window.addEventListener("scroll", handleScroll);

function updateUI(data) {
  function updateControlValue(
    controlId,
    dataValue,
    maxTrack,
    thumb,
    handleMoveFunction,
    factor
  ) {
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

  function updateInputs(controlId, dataValue) {
    if (dataValue !== undefined) {
      const inputs = document.getElementById(controlId);
      inputs.value = dataValue;
    }
  }

  updateDropdownList("selected-item-modes", data.MODE);
  updateDropdownList("selected-item-animations", data.ANIMATION);
  updateDropdownList("selected-item-presetColors", data.LED_COLOR_PRESET);
  updateDropdownList("selected-item-colorOrder", data.LED_COLOR_ORDER);

  updateControlValue("HUE", data.HUE, track, thumb, handleMove, 255);
  updateControlValue(
    "SATURATION",
    data.SATURATION,
    saturationTrack,
    saturationThumb,
    handleSaturationMove,
    255
  );
  updateControlValue(
    "BRIGHTNESS",
    data.BRIGHTNESS,
    brightnessTrack,
    brightnessThumb,
    handleBrightnessMove,
    255
  );
  updateControlValue(
    "FADE",
    data.FADE,
    fadeTrack,
    fadeThumb,
    handleFadeMove,
    255
  );
  updateControlValue(
    "SPLASH",
    data.SPLASH,
    splashTrack,
    splashThumb,
    handleSplashMove,
    16
  );
  updateControlValue("BG", data.BG, bgTrack, bgThumb, handleBgMove, 255);
  updateControlValue(
    "SPLIT",
    data.SPLIT,
    splitTrack,
    splitThumb,
    handleSplitMove,
    100
  );

  updateToggles("cb1-8", data.FIX_TOGGLE);
  updateToggles("cb2-8", data.BG_TOGGLE);
  updateToggles("cb3-8", data.REVERSE_TOGGLE);
  updateToggles("cb4-8", data.BGUPDATE_TOGGLE);

  updateInputs("maxCurrent", data.LED_CURRENT);
  updateInputs("ledDataPin", data.LED_PIN);
}

// DropdownList script for LED Modes
const selectedItemModes = document.querySelector("#selected-item-modes");

selectedItemModes.addEventListener("change", () => {
  const selectedModeId = selectedItemModes.value;
  console.log("Selected Mode ID:", selectedModeId); // Debugging statement

  // Send a WebSocket message for changing the LED mode
  sendData("ChangeLEDModeAction", { mode: selectedModeId });
  sendData("RequestValues");
});

// DropdownList script for Animations
const selectedItemAnimations = document.querySelector(
  "#selected-item-animations"
);

selectedItemAnimations.addEventListener("change", () => {
  const selectedAnimationId = selectedItemAnimations.value;
  console.log("Selected Animation ID:", selectedAnimationId); // Debugging statement

  // Send a WebSocket message for changing the animation
  sendData("ChangeAnimationAction", { animation: selectedAnimationId });
});

// DropdownList script for LED Strip Color Order
const selectedItemColorOrder = document.querySelector(
  "#selected-item-colorOrder"
);

selectedItemColorOrder.addEventListener("change", () => {
  const selectedColorOrderId = selectedItemColorOrder.value;
  console.log("Selected Color Order ID:", selectedColorOrderId); // Debugging statement

  // Send a WebSocket message for changing the animation
  sendData("ChangeColorOrderAction", { colorOrder: selectedColorOrderId });

  // Refresh the page after 2 seconds
  setTimeout(() => {
    location.reload();
  }, 3500);
});

// DropdownList script for Color Preset
const selectedItemPresetColors = document.querySelector(
  "#selected-item-presetColors"
);
// Map preset IDs to HSB values
const presetColorsHSB = [
  [0, 255, 255], // Red
  [90, 255, 255], // Green
  [160, 255, 255], // Blue
  [0, 0, 255], // White
  [35, 255, 255], // Yellow
  [10, 255, 255], // Orange
  [205, 255, 255], // Purple
  [240, 190, 255], // Pink
  [150, 150, 255], // Teal
  [80, 255, 255], // Lime
  [130, 170, 255], // Cyan
  [245, 255, 255], // Magenta
  [252, 190, 255], // Peach
  [210, 200, 255], // Lavender
  [130, 150, 255], // Turquoise
  [22, 255, 255] // Gold
];

selectedItemPresetColors.addEventListener("change", () => {
  const selectedPresetColorId = selectedItemPresetColors.value;
  console.log("Selected Preset Color ID:", selectedPresetColorId); // Debugging statement

  // Get HSB values based on the selected preset ID
  const [hue, saturation, brightness] = presetColorsHSB[selectedPresetColorId];

  // Log the HSB values before sending them through WebSocket messages
  console.log("Sending HSB values:", { hue, saturation, brightness });

  // Send WebSocket message for changing HSB values
  sendData("ColorPresetAction", {
    colorPresetHue: hue,
    colorPresetSaturation: saturation,
    colorPresetID: selectedPresetColorId
  });

  sendData("RequestValues");

  // Update UI elements based on the selected preset
  updateUIElements(hue, saturation);
  handleEnd(selectedItemPresetColors);
  updateBrightnessTrackGradient(hue);
  updateSaturationTrackGradient(hue);
});

// Function to update UI elements based on the selected preset
function updateUIElements(hue, saturation) {
  // Mapping hue from the range of 0-255 to 0-360
  const mappedHue = (hue / 255) * 360;

  // Setting thumb position based on mapped hue
  const newHuePosition =
    (mappedHue / 360) * (track.offsetWidth - thumb.offsetWidth);
  thumb.style.left = newHuePosition + "px";

  // Setting saturation thumb position based on saturation value
  const newSaturationPosition =
    (saturation / 255) *
    (saturationTrack.offsetWidth - saturationThumb.offsetWidth);
  saturationThumb.style.left = newSaturationPosition + "px";

  // Setting background color using mapped hue
  thumb.style.backgroundColor = `hsl(${mappedHue}, 100%, 50%)`;
}

// Function to handle the end of the interaction
function handleEnd(inputElement) {
  // Trigger the input event manually on the provided input element
  const inputEvent = new Event("input", {
    bubbles: true,
    cancelable: true
  });
  inputElement.dispatchEvent(inputEvent);

  // Check the state of the BGAction checkbox
  const bgCheckbox = document.getElementById("cb2-8"); // Adjust the ID accordingly
  const bgupdateCheckbox = document.getElementById("cb4-8"); // Adjust the ID accordingly
  if (bgCheckbox.checked && bgupdateCheckbox.checked) {
    bgCheckbox.checked = false;
    bgCheckbox.click();
  }
}

// Hue Slider Code
const thumb = document.querySelector(".thumb");
const track = document.querySelector(".track");
const hueValueInput = document.getElementById("HUE");
let isDragging = false;

// Function to handle mouse and touch move
function handleMove(xPosition) {
  const maxPosition = track.offsetWidth - thumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  thumb.style.left = xPosition + "px";

  // Calculate the hue value based on thumb position
  const hue = (xPosition / maxPosition) * 360;

  // Map the hue value to the 0-255 range for the slider
  const mappedHue = Math.round((hue / 360) * 255);
  hueValueInput.value = mappedHue; // Store the mapped hue value in the hidden input

  // Set the thumb's background color based on the hue value
  thumb.style.backgroundColor = `hsl(${hue}, 100%, 50%)`;

  // Set Color Preset to Custom
  let foundMatchingHue = false;

  // Check if mapped hue matches any preset hue
  for (let i = 0; i < presetColorsHSB.length; i++) {
    const presetHue = presetColorsHSB[i][0];
    if (presetHue === mappedHue) {
      foundMatchingHue = true;
      break;
    }
  }

  if (!foundMatchingHue) {
    console.log("HSB doesn't match, setting ColorPreset to Custom!");
    selectedItemPresetColors.value = 16; // Set the value to 16
  }
}

// Mouse drag
thumb.addEventListener("mousedown", e => {
  isDragging = true;
  thumb.style.transition = "none";
  const offsetX = e.clientX - thumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isDragging) return;

    const newPosition =
      e.clientX - track.getBoundingClientRect().left - offsetX;
    handleMove(newPosition);
  }

  function onMouseUp() {
    isDragging = false;
    handleEnd(hueValueInput);

    thumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices
thumb.addEventListener("touchstart", e => {
  isDragging = true;
  thumb.style.transition = "none";
  const offsetX = e.touches[0].clientX - thumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isDragging) return;

    const newPosition =
      e.touches[0].clientX - track.getBoundingClientRect().left - offsetX;
    handleMove(newPosition);
  }

  function onTouchEnd() {
    isDragging = false;
    handleEnd(hueValueInput);

    thumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click
track.addEventListener("click", e => {
  const clickX = e.clientX - track.getBoundingClientRect().left;
  const thumbPosition = clickX - thumb.offsetWidth / 2;
  handleMove(thumbPosition);
  handleEnd(hueValueInput);
  // Restore smooth transition for the thumb
  thumb.style.transition = "left 0.3s ease";
});

// Saturation slider
const saturationThumb = document.querySelector(".saturation-slider .thumb");
const saturationTrack = document.querySelector(".saturation-slider .track");
const saturationValueInput = document.getElementById("SATURATION");
let isSaturationDragging = false;

// Function to update the saturation track gradient based on the hue value and current saturation level
function updateSaturationTrackGradient(hue) {
  const track = document.querySelector(".saturation-slider .track");
  // Map the hue value from 0-360 to 0-255
  const mappedHue = Math.round((hue / 180) * 255);
  // Get the current saturation level
  const saturation = parseInt(saturationValueInput.value);
  // Update the track gradient CSS with the mapped hue value and current saturation level
  track.style.background = `linear-gradient(to right, #fff, hsl(${mappedHue}, 100%, 50%))`;
}
// Function to handle mouse and touch move
function handleSaturationMove(xPosition) {
  const maxPosition = saturationTrack.offsetWidth - saturationThumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  saturationThumb.style.left = xPosition + "px";

  // Calculate the saturation value based on thumb position
  const saturation = Math.round((xPosition / maxPosition) * 255);

  saturationValueInput.value = saturation;
  updateSaturationTrackGradient(hueValueInput.value);

  // Set Color Preset to Custom
  let foundMatchingSaturation = false;

  // Check if saturation matches any preset saturation
  for (let i = 0; i < presetColorsHSB.length; i++) {
    const presetSaturation = presetColorsHSB[i][1];
    if (presetSaturation === saturation) {
      foundMatchingSaturation = true;
      break;
    }
  }

  if (!foundMatchingSaturation) {
    console.log("HSB doesn't match, setting ColorPreset to Custom!");
    selectedItemPresetColors.value = 16; // Set the value to 16
  }
}

// Mouse drag
saturationThumb.addEventListener("mousedown", e => {
  isSaturationDragging = true;
  saturationThumb.style.transition = "none";
  const offsetX = e.clientX - saturationThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isSaturationDragging) return;

    const newPosition =
      e.clientX - saturationTrack.getBoundingClientRect().left - offsetX;
    handleSaturationMove(newPosition);
  }

  function onMouseUp() {
    isSaturationDragging = false;
    handleEnd(saturationValueInput);
    saturationThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices
saturationThumb.addEventListener("touchstart", e => {
  isSaturationDragging = true;
  saturationThumb.style.transition = "none";
  const offsetX =
    e.touches[0].clientX - saturationThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isSaturationDragging) return;

    const newPosition =
      e.touches[0].clientX -
      saturationTrack.getBoundingClientRect().left -
      offsetX;
    handleSaturationMove(newPosition);
  }

  function onTouchEnd() {
    isSaturationDragging = false;
    handleEnd(saturationValueInput);
    saturationThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click
saturationTrack.addEventListener("click", e => {
  const clickX = e.clientX - saturationTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - saturationThumb.offsetWidth / 2;
  handleSaturationMove(thumbPosition);
  handleEnd(saturationValueInput);
  // Restore smooth transition for the thumb
  saturationThumb.style.transition = "left 0.3s ease";
});

// Set the initial background brightness
const initialSaturation = parseInt(saturationValueInput.value);

// Calculate the initial thumb position based on the initial value
const initialSaturationThumbPosition =
  (initialSaturation / 255) *
  (saturationTrack.offsetWidth - saturationThumb.offsetWidth);
saturationThumb.style.left = initialSaturationThumbPosition + "px";
updateSaturationTrackGradient(hueValueInput.value);

// Brightness Slider Code
const brightnessThumb = document.querySelector(".brightness-slider .thumb");
const brightnessTrack = document.querySelector(".brightness-slider .track");
const brightnessValueInput = document.getElementById("BRIGHTNESS");
let isBrightnessDragging = false;

// Function to update the brightness slider's track gradient based on the hue value
function updateBrightnessTrackGradient(hue) {
  const track = document.querySelector(".brightness-slider .track");
  // Map the hue value from 0-360 to;
  0 - 255;
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

  brightnessThumb.style.left = xPosition + "px";

  // Calculate the brightness value based on thumb position
  const brightness = Math.round((xPosition / maxPosition) * maxBrightness);

  // Update the brightness value in the hidden input
  brightnessValueInput.value = brightness;
}

// Update the brightness slider's track gradient when the hue slider value changes
hueValueInput.addEventListener("input", () => {
  const hue = parseInt(hueValueInput.value);
  updateBrightnessTrackGradient(hue);
  updateSaturationTrackGradient(hue);
});

brightnessThumb.addEventListener("mousedown", e => {
  isBrightnessDragging = true;
  brightnessThumb.style.transition = "none";
  const offsetX = e.clientX - brightnessThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onBrightnessMouseMove);
  document.addEventListener("mouseup", onBrightnessMouseUp);

  function onBrightnessMouseMove(e) {
    if (!isBrightnessDragging) return;

    const newPosition =
      e.clientX - brightnessTrack.getBoundingClientRect().left - offsetX;
    handleBrightnessMove(newPosition);
  }

  function onBrightnessMouseUp() {
    isBrightnessDragging = false;
    handleEnd(brightnessValueInput);
    brightnessThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onBrightnessMouseMove);
    document.removeEventListener("mouseup", onBrightnessMouseUp);
  }
});

brightnessThumb.addEventListener("touchstart", e => {
  isBrightnessDragging = true;
  brightnessThumb.style.transition = "none";
  const offsetX =
    e.touches[0].clientX - brightnessThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onBrightnessTouchMove);
  document.addEventListener("touchend", onBrightnessTouchEnd);

  function onBrightnessTouchMove(e) {
    if (!isBrightnessDragging) return;

    const newPosition =
      e.touches[0].clientX -
      brightnessTrack.getBoundingClientRect().left -
      offsetX;
    handleBrightnessMove(newPosition);
  }

  function onBrightnessTouchEnd() {
    isBrightnessDragging = false;
    handleEnd(brightnessValueInput);
    brightnessThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onBrightnessTouchMove);
    document.removeEventListener("touchend", onBrightnessTouchEnd);
  }
});

brightnessTrack.addEventListener("click", e => {
  const clickX = e.clientX - brightnessTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - brightnessThumb.offsetWidth / 2;
  handleBrightnessMove(thumbPosition);
  handleEnd(brightnessValueInput);
  // Restore smooth transition for the brightness thumb
  brightnessThumb.style.transition = "left 0.3s ease";
});

// Initial setup for the brightness slider
const initialBrightness = parseInt(brightnessValueInput.value);
const maxBrightness = 255; // Maximum brightness value
const initialBrightnessPosition =
  (initialBrightness / maxBrightness) *
  (brightnessTrack.offsetWidth - brightnessThumb.offsetWidth);
brightnessThumb.style.left = initialBrightnessPosition + "px";
// Initialize the brightness slider's track gradient based on the initial hue
updateBrightnessTrackGradient(hueValueInput.value);

// Fade slider
const fadeThumb = document.querySelector(".fade-slider .thumb");
const fadeTrack = document.querySelector(".fade-slider .track");
const fadeValueInput = document.getElementById("FADE");
let isFadeDragging = false;

// Function to handle mouse and touch move
function handleFadeMove(xPosition) {
  const maxPosition = fadeTrack.offsetWidth - fadeThumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  fadeThumb.style.left = xPosition + "px";

  // Calculate the fade value based on thumb position
  const fade = Math.round((xPosition / maxPosition) * 255);

  fadeValueInput.value = fade;
}

// Mouse drag
fadeThumb.addEventListener("mousedown", e => {
  isFadeDragging = true;
  fadeThumb.style.transition = "none";
  const offsetX = e.clientX - fadeThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isFadeDragging) return;

    const newPosition =
      e.clientX - fadeTrack.getBoundingClientRect().left - offsetX;
    handleFadeMove(newPosition);
  }

  function onMouseUp() {
    isFadeDragging = false;
    handleEnd(fadeValueInput);
    fadeThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices
fadeThumb.addEventListener("touchstart", e => {
  isFadeDragging = true;
  fadeThumb.style.transition = "none";
  const offsetX = e.touches[0].clientX - fadeThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isFadeDragging) return;

    const newPosition =
      e.touches[0].clientX - fadeTrack.getBoundingClientRect().left - offsetX;
    handleFadeMove(newPosition);
  }

  function onTouchEnd() {
    isFadeDragging = false;
    handleEnd(fadeValueInput);
    fadeThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click
fadeTrack.addEventListener("click", e => {
  const clickX = e.clientX - fadeTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - fadeThumb.offsetWidth / 2;
  handleFadeMove(thumbPosition);
  handleEnd(fadeValueInput);
  // Restore smooth transition for the thumb
  fadeThumb.style.transition = "left 0.3s ease";
});

// Splash slider
const splashThumb = document.querySelector(".splash-slider .thumb");
const splashTrack = document.querySelector(".splash-slider .track");
const splashValueInput = document.getElementById("SPLASH");
let isSplashDragging = false;

// Function to handle mouse and touch move
function handleSplashMove(xPosition) {
  const maxPosition = splashTrack.offsetWidth - splashThumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  splashThumb.style.left = xPosition + "px";

  // Calculate the splash value based on thumb position
  const splash = Math.round((xPosition / maxPosition) * 16);

  splashValueInput.value = splash;
}

// Mouse drag
splashThumb.addEventListener("mousedown", e => {
  isSplashDragging = true;
  splashThumb.style.transition = "none";
  const offsetX = e.clientX - splashThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isSplashDragging) return;

    const newPosition =
      e.clientX - splashTrack.getBoundingClientRect().left - offsetX;
    handleSplashMove(newPosition);
  }

  function onMouseUp() {
    isSplashDragging = false;
    handleEnd(splashValueInput);
    splashThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices
splashThumb.addEventListener("touchstart", e => {
  isSplashDragging = true;
  splashThumb.style.transition = "none";
  const offsetX =
    e.touches[0].clientX - splashThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isSplashDragging) return;

    const newPosition =
      e.touches[0].clientX - splashTrack.getBoundingClientRect().left - offsetX;
    handleSplashMove(newPosition);
  }

  function onTouchEnd() {
    isSplashDragging = false;
    handleEnd(splashValueInput);
    splashThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click
splashTrack.addEventListener("click", e => {
  const clickX = e.clientX - splashTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - splashThumb.offsetWidth / 2;
  handleSplashMove(thumbPosition);
  handleEnd(splashValueInput);
  // Restore smooth transition for the thumb
  splashThumb.style.transition = "left 0.3s ease";
});

const initialSplash = parseInt(splashValueInput.value);
const maxSplash = 16;
const initialSplashPosition =
  (initialSplash / maxSplash) *
  (splashTrack.offsetWidth - splashThumb.offsetWidth);
splashThumb.style.left = initialSplashPosition + "px";

// Background slider
const bgThumb = document.querySelector(".bg-slider .thumb");
const bgTrack = document.querySelector(".bg-slider .track");
const bgValueInput = document.getElementById("BG");
let isBgDragging = false;

// Function to handle mouse and touch move for background slider
function handleBgMove(xPosition) {
  const maxPosition = bgTrack.offsetWidth - bgThumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  bgThumb.style.left = xPosition + "px";

  // Calculate the background brightness value based on thumb position
  const brightness = Math.round((xPosition / maxPosition) * 255);

  bgValueInput.value = brightness;
}

// Mouse drag for background slider
bgThumb.addEventListener("mousedown", e => {
  isBgDragging = true;
  bgThumb.style.transition = "none";
  const offsetX = e.clientX - bgThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isBgDragging) return;

    const newPosition =
      e.clientX - bgTrack.getBoundingClientRect().left - offsetX;
    handleBgMove(newPosition);
  }

  function onMouseUp() {
    isBgDragging = false;
    handleEnd(bgValueInput);
    bgThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices for background slider
bgThumb.addEventListener("touchstart", e => {
  isBgDragging = true;
  bgThumb.style.transition = "none";
  const offsetX = e.touches[0].clientX - bgThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isBgDragging) return;

    const newPosition =
      e.touches[0].clientX - bgTrack.getBoundingClientRect().left - offsetX;
    handleBgMove(newPosition);
  }

  function onTouchEnd() {
    isBgDragging = false;
    handleEnd(bgValueInput);
    bgThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click for background slider
bgTrack.addEventListener("click", e => {
  const clickX = e.clientX - bgTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - bgThumb.offsetWidth / 2;
  handleBgMove(thumbPosition);
  handleEnd(bgValueInput);
  // Restore smooth transition for the thumb
  bgThumb.style.transition = "left 0.3s ease";
});

// Set the initial background brightness
const initialBgBrightness = parseInt(bgValueInput.value);

// Calculate the initial thumb position based on the initial value
const initialBGBrightnessThumbPosition =
  (initialBgBrightness / 255) * (bgTrack.offsetWidth - bgThumb.offsetWidth);
bgThumb.style.left = initialBGBrightnessThumbPosition + "px";

// Split slider
const splitThumb = document.querySelector(".split-slider .thumb");
const splitTrack = document.querySelector(".split-slider .track");
const splitValueInput = document.getElementById("SPLIT");
let isSplitDragging = false;

// Function to handle mouse and touch move for split slider
function handleSplitMove(xPosition) {
  const maxPosition = splitTrack.offsetWidth - splitThumb.offsetWidth;
  const minPosition = 0;

  // Ensure the thumb stays within the track bounds
  xPosition = Math.max(minPosition, Math.min(xPosition, maxPosition));

  splitThumb.style.left = xPosition + "px";

  // Calculate the split value based on thumb position
  const splitValue = Math.round((xPosition / maxPosition) * 100);

  splitValueInput.value = splitValue;
}

// Mouse drag for split slider
splitThumb.addEventListener("mousedown", e => {
  isSplitDragging = true;
  splitThumb.style.transition = "none";
  const offsetX = e.clientX - splitThumb.getBoundingClientRect().left;

  document.addEventListener("mousemove", onMouseMove);
  document.addEventListener("mouseup", onMouseUp);

  function onMouseMove(e) {
    if (!isSplitDragging) return;

    const newPosition =
      e.clientX - splitTrack.getBoundingClientRect().left - offsetX;
    handleSplitMove(newPosition);
  }

  function onMouseUp() {
    isSplitDragging = false;
    handleEnd(splitValueInput);
    splitThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("mousemove", onMouseMove);
    document.removeEventListener("mouseup", onMouseUp);
  }
});

// Touch event handling for mobile devices for split slider
splitThumb.addEventListener("touchstart", e => {
  isSplitDragging = true;
  splitThumb.style.transition = "none";
  const offsetX =
    e.touches[0].clientX - splitThumb.getBoundingClientRect().left;

  document.addEventListener("touchmove", onTouchMove);
  document.addEventListener("touchend", onTouchEnd);

  function onTouchMove(e) {
    if (!isSplitDragging) return;

    const newPosition =
      e.touches[0].clientX - splitTrack.getBoundingClientRect().left - offsetX;
    handleSplitMove(newPosition);
  }

  function onTouchEnd() {
    isSplitDragging = false;
    handleEnd(splitValueInput);
    splitThumb.style.transition = "left 0.3s ease"; // Restore smooth transition
    document.removeEventListener("touchmove", onTouchMove);
    document.removeEventListener("touchend", onTouchEnd);
  }
});

// Track click for split slider
splitTrack.addEventListener("click", e => {
  const clickX = e.clientX - splitTrack.getBoundingClientRect().left;
  const thumbPosition = clickX - splitThumb.offsetWidth / 2;
  handleSplitMove(thumbPosition);
  handleEnd(splitValueInput);
  // Restore smooth transition for the thumb
  splitThumb.style.transition = "left 0.3s ease";
});

// Set the initial split
const initialSplit = parseInt(splitValueInput.value);

// Calculate the initial thumb position based on the initial value
const initialSplitThumbPosition =
  (initialSplit / 100) * (splitTrack.offsetWidth - splitThumb.offsetWidth);
splitThumb.style.left = initialSplitThumbPosition + "px";

function handleWindowResize() {
  const maxHuePosition = track.offsetWidth - thumb.offsetWidth;
  const maxSaturationPosition =
    saturationTrack.offsetWidth - saturationThumb.offsetWidth;
  const maxBrightnessPosition =
    brightnessTrack.offsetWidth - brightnessThumb.offsetWidth;
  const maxFadePosition = fadeTrack.offsetWidth - fadeThumb.offsetWidth;
  const maxSplashPosition = splashTrack.offsetWidth - splashThumb.offsetWidth;
  const maxBGPosition = bgTrack.offsetWidth - bgThumb.offsetWidth;
  const maxSplitPosition = splitTrack.offsetWidth - splitThumb.offsetWidth;

  const mappedHue = (parseInt(hueValueInput.value) / 255) * 360;
  const mappedSaturation = parseInt(saturationValueInput.value);
  const mappedBrightness = parseInt(brightnessValueInput.value);
  const mappedFade = parseInt(fadeValueInput.value);
  const mappedSplash = parseInt(splashValueInput.value);
  const mappedBG = parseInt(bgValueInput.value);
  const mappedSplit = parseInt(splitValueInput.value);

  // Calculate the new position based on the mapped hue value
  const newPosition = (mappedHue / 360) * maxHuePosition;
  const newSaturationPosition =
    (mappedSaturation / 255) * maxSaturationPosition;
  const newBrightnessPosition =
    (mappedBrightness / 255) * maxBrightnessPosition;
  const newFadePosition = (mappedFade / 255) * maxFadePosition;
  const newSplashPosition = (mappedSplash / 16) * maxSplashPosition;
  const newBGPosition = (mappedBG / 255) * maxBGPosition;
  const newSplitPosition = (mappedSplit / 100) * maxSplitPosition;

  // Update the thumb's position
  thumb.style.left = newPosition + "px";
  saturationThumb.style.left = newSaturationPosition + "px";
  brightnessThumb.style.left = newBrightnessPosition + "px";
  fadeThumb.style.left = newFadePosition + "px";
  splashThumb.style.left = newSplashPosition + "px";
  bgThumb.style.left = newBGPosition + "px";
  splitThumb.style.left = newSplitPosition + "px";

  // Calculate the background color based on the hue value
  const backgroundColor = `hsl(${mappedHue}, 100%, 50%)`;

  // Update the thumb's background color
  thumb.style.backgroundColor = backgroundColor;
  thumb.style.transition = "left 0s ease";
  saturationThumb.style.transition = "left 0s ease";
  brightnessThumb.style.transition = "left 0s ease";
  fadeThumb.style.transition = "left 0s ease";
  splashThumb.style.transition = "left 0s ease";
  bgThumb.style.transition = "left 0s ease";
  splitThumb.style.transition = "left 0s ease";
}
// Add the event listener for window resize
window.addEventListener("resize", handleWindowResize);

// Step 1: Create an SVG element
var svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
svg.setAttribute("height", "100%"); // Set the SVG width to 100% of its container
svg.setAttribute("width", "100%"); // Set the SVG width to 100% of its container

// Step 2: Append the SVG element to the div
var div = document.getElementById("pianoKeysVisuals");
div.appendChild(svg);

// Calculate the width for each rectangle
var totalRectangles = 52;
var rectangleWidth = 100 / totalRectangles; // 100% divided by the number of rectangles

// Step 3: Create and append 52 white rectangles
for (var i = 0; i < totalRectangles; i++) {
  // Create rectangle
  var rect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
  rect.setAttribute("width", rectangleWidth + "%");
  rect.setAttribute("height", "100%");
  rect.setAttribute("fill", "white");
  rect.setAttribute("stroke", "black");
  rect.setAttribute("stroke-width", "1");
  rect.setAttribute("rx", "3"); // Set border-radius for rounded corners

  // Move each white rectangle to its correct position
  rect.setAttribute("x", i * rectangleWidth + "%");

  // Append the rectangle to the SVG element
  svg.appendChild(rect);
}

// Step 4: Create a black rectangle
var blackRect = document.createElementNS("http://www.w3.org/2000/svg", "rect");
blackRect.setAttribute("width", rectangleWidth / 1.25 + "%");
blackRect.setAttribute("height", "70%"); // Half of the height of a white key
blackRect.setAttribute("fill", "black");
blackRect.setAttribute("stroke", "black");
blackRect.setAttribute("stroke-width", "1");
blackRect.setAttribute("rx", "3"); // Set border-radius for rounded corners

// Position the black key between the center of the first and second white keys
blackRect.setAttribute("x", rectangleWidth / 1.75 + "%");

// Append the black rectangle to the SVG element
svg.appendChild(blackRect);

// Define the positions for black keys across 7 octaves
var blackKeyPositions = [
  1.05,
  2.15,
  4.05,
  5.1,
  6.15, // First octave
  8.05,
  9.1,
  11.05,
  12.1,
  13.15, // Second octave
  15.05,
  16.1,
  18.05,
  19.1,
  20.15, // Third octave
  22.05,
  23.1,
  25.05,
  26.1,
  27.15, // Fourth octave
  29.05,
  30.1,
  32.05,
  33.1,
  34.15, // Fifth octave
  36.05,
  37.1,
  39.05,
  40.1,
  41.15, // Sixth octave
  43.05,
  44.1,
  46.05,
  47.1,
  48.15 // Seventh octave
];
var patternX = 4; // Adjust this value as needed

// Create and append black rectangles based on the positions
for (var i = 0; i < blackKeyPositions.length; i++) {
  var position = blackKeyPositions[i];

  // Create black rectangle
  var blackRect = document.createElementNS(
    "http://www.w3.org/2000/svg",
    "rect"
  );
  blackRect.setAttribute("width", rectangleWidth / 1.25 + "%");
  blackRect.setAttribute("height", "70%"); // Half of the height of a white key
  blackRect.setAttribute("fill", "black");
  blackRect.setAttribute("stroke", "black");
  blackRect.setAttribute("stroke-width", "1");
  blackRect.setAttribute("rx", "3"); // Set border-radius for rounded corners

  // Position the black key
  blackRect.setAttribute(
    "x",
    patternX + position * rectangleWidth - rectangleWidth / 1.75 + "%"
  );

  // Append the black rectangle to the SVG element
  svg.appendChild(blackRect);
}
// Step 4: Create two rectangles for highlighting
const highlightRect1 = document.createElementNS(
  "http://www.w3.org/2000/svg",
  "rect"
);
highlightRect1.setAttribute("width", "0%");
highlightRect1.setAttribute("height", "100%");
highlightRect1.setAttribute("fill", "rgba(32, 32, 32, 0.8)"); // Red color with 50% opacity

const highlightRect2 = document.createElementNS(
  "http://www.w3.org/2000/svg",
  "rect"
);
highlightRect2.setAttribute("width", "0%");
highlightRect2.setAttribute("height", "100%");
highlightRect2.setAttribute("fill", "rgba(32, 32, 32, 0.8)"); // Blue color with 50% opacity

const highlightRectLeft = document.createElementNS(
  "http://www.w3.org/2000/svg",
  "rect"
);
highlightRectLeft.setAttribute("height", "10%");
highlightRectLeft.setAttribute("fill", "rgba(255, 0, 0, 1)"); // Red color with 100% opacity

const highlightRectRight = document.createElementNS(
  "http://www.w3.org/2000/svg",
  "rect"
);
highlightRectRight.setAttribute("height", "10%");
highlightRectRight.setAttribute("fill", "rgba(0, 0, 255, 1)"); // Blue color with 100% opacity

// Append the highlight rectangles to the SVG element
svg.appendChild(highlightRectLeft);
svg.appendChild(highlightRectRight);

// Set the initial ratio
let splitRatio = 50; // in percentage

// Update the widths and positions based on the ratio
function updateHighlightRects() {
  const totalWidth = 100; // Total width of the container

  const leftWidth = (totalWidth * splitRatio) / 100;
  const rightWidth = totalWidth - leftWidth;

  highlightRectLeft.setAttribute("width", leftWidth + "%");
  highlightRectRight.setAttribute("width", rightWidth + "%");

  // Update the position of the right rectangle
  highlightRectRight.setAttribute("x", leftWidth + "%");
}

document.getElementById("SPLIT").addEventListener("input", function() {
  splitRatio = this.value;
  updateHighlightRects();
});

updateHighlightRects();

// Append the highlight rectangles to the SVG element
svg.appendChild(highlightRect1);
svg.appendChild(highlightRect2);

function addButtonHSBListener(controlId, actionName, index) {
  document.getElementById(controlId).addEventListener("click", function() {
    const hue = hueValueInput.value;
    const saturation = saturationValueInput.value;
    const brightness = brightnessValueInput.value;

    const mappedHue = (hueValueInput.value / 255) * 360; // 0-255 range for hue, converted to 0-360 degrees
    const mappedSaturation = (saturationValueInput.value / 255) * 100; // 0-255 range for saturation, converted to 0-100%
    const mappedBrightness = (brightnessValueInput.value / 255) * 50; // 0-255 range for brightness, converted to 0-50% (50% lightness)

    const highlightRect = index === 0 ? highlightRectLeft : highlightRectRight;
    highlightRect.setAttribute(
      "fill",
      `hsl(${mappedHue}, ${mappedSaturation}%, ${mappedBrightness}%)`
    );

    // Send data with hue, saturation, brightness, and actionName
    sendData(actionName, {
      hue: hue,
      saturation: saturation,
      brightness: brightness,
      index: index
    });
  });
}

// Example usage:
addButtonHSBListener("SETLEFTSPLIT", "SetSplitAction", 0); // Left split
addButtonHSBListener("SETRIGHTSPLIT", "SetSplitAction", 1); // Right split

function createButtonListener(button, values, index, actionName) {
  button.addEventListener("click", function() {
    index = (index + 1) % values.length;
    button.textContent = values[index];
    sendData(actionName, { value: index });

    // Check if the clicked button has the specific ID "PianoSize"
    if (button.id === "PianoSize") {
      // Update the position and width of the highlight rectangles based on the button state
      updateHighlightRectangles(index);
    }

    console.log("Sending:", actionName + index); // Debugging
  });
}

function simpleActionButton(button, actionName) {
  button.addEventListener("click", function() {
    sendData(actionName);
    console.log("Sending:", actionName); // Debugging
  });
}

const scanBluetoothButton = document.getElementById("ScanBluetooth");
simpleActionButton(scanBluetoothButton, "ScanBluetoothAction");

// Function to update highlight rectangles based on the button state
function updateHighlightRectangles(sizeIndex) {
  // Adjust the width and x position based on the button state
  switch (sizeIndex) {
    case 0: // "88 Key"
      highlightRect1.setAttribute("width", "0%"); // Customize width for this case
      highlightRect1.setAttribute("x", "0%"); // Customize x position for this case
      highlightRect2.setAttribute("width", "0%");
      highlightRect2.setAttribute("x", "0%");
      break;
    case 1: // "76 Key"
      highlightRect1.setAttribute("width", "7.8%"); // Customize width for this case
      highlightRect1.setAttribute("x", "0%"); // Customize x position for this case
      highlightRect2.setAttribute("width", "7.8%");
      highlightRect2.setAttribute("x", "94.2%");
      break;
    case 2: // "73 Key"
      highlightRect1.setAttribute("width", "7.8%"); // Customize width for this case
      highlightRect1.setAttribute("x", "0%"); // Customize x position for this case
      highlightRect2.setAttribute("width", "9.6%");
      highlightRect2.setAttribute("x", "90.4%");
      break;
    case 3: // "61 Key"
      highlightRect1.setAttribute("width", "17.4%"); // Customize width for this case
      highlightRect1.setAttribute("x", "0%"); // Customize x position for this case
      highlightRect2.setAttribute("width", "13.5%");
      highlightRect2.setAttribute("x", "86.5%");
      break;
    case 4: // "49 Key"
      highlightRect1.setAttribute("width", "17.4%"); // Customize width for this case
      highlightRect1.setAttribute("x", "0%"); // Customize x position for this case
      highlightRect2.setAttribute("width", "27%");
      highlightRect2.setAttribute("x", "73%");
      break;
  }
}

const sizes = ["88 Key", "76 Key", "73 Key", "61 Key", "49 Key"];
let sizeIndex = 0;
const pianoSizeButton = document.getElementById("PianoSize");
createButtonListener(pianoSizeButton, sizes, sizeIndex, "PianoSizeAction");

const ratios = ["1:2", "1:1"];
let ratioIndex = 0;
const ledScaleRatioButton = document.getElementById("LEDScaleRatio");
createButtonListener(
  ledScaleRatioButton,
  ratios,
  ratioIndex,
  "LedScaleRatioAction"
);

function createCheckboxListener(checkbox, actionName) {
  checkbox.addEventListener("change", function() {
    const value = this.checked ? 1 : 0;
    sendData(actionName, { value });
    console.log("Sending:", actionName + value); // Debugging
  });
}

const cb2Checkbox1 = document.getElementById("cb1-8");
const cb2Checkbox2 = document.getElementById("cb2-8");
const cb2Checkbox3 = document.getElementById("cb3-8");
const cb2Checkbox4 = document.getElementById("cb4-8");
const cb2Checkbox5 = document.getElementById("cb5-8");

createCheckboxListener(cb2Checkbox1, "FixAction");
createCheckboxListener(cb2Checkbox2, "BGAction");
createCheckboxListener(cb2Checkbox3, "DirectionAction");
createCheckboxListener(cb2Checkbox4, "BGUpdateAction");

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
    const minValue = 450;
    const maxValue = 3000;
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
    sendData("CurrentAction", { value: enteredValue });
    console.log("Sending:", "CurrentAction", enteredValue);
  }, typingTimeout);
});

const ledDataPinInput = document.getElementById("ledDataPin");

// Function to display a warning when the input field is focused
ledDataPinInput.addEventListener("focus", function() {
  if (!stripLEDalertShown) {
    alert(
      "Enter valid GPIO pin number on your ESP32 S2/S3 you want to use for W2812 LED Strip Data"
    );
    stripLEDalertShown = true;
  }
});

ledDataPinInput.addEventListener("input", function() {
  clearTimeout(typingTimer);
  typingTimer = setTimeout(function() {
    const minValue = 0;
    const maxValue = 50;
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

    // Display alert after updating the input value
    alert("ESP32 will now restart.");

    // Send the action and entered value to the socket
    sendData("LedDataPinAction", { value: enteredValue });
    console.log("Sending:", "LedDataPinAction", enteredValue);

    // Refresh the page after 2 seconds
    setTimeout(function() {
      location.reload();
    }, 3500);
  }, typingTimeout);
});

function showUpdateMenu() {
  // Define handleMessage function here
  function handleMessage(event) {
    var data = JSON.parse(event.data);

    // Send a request to fetch the latest release tag from the GitHub repository
    fetchLatestReleaseTag(data)
      .then(latestReleaseTag => {
        // Assuming data.FirmwareVersion holds the firmware version
        const firmwareVersion = data.FirmwareVersion;

        // Compare the firmware version with the latest release tag
        if (firmwareVersion === latestReleaseTag) {
          window.alert(
            `ESP32 Board Firmware ${firmwareVersion} matches with latest PianoLux Firmware ${latestReleaseTag} version. No need to update!`
          );
        } else {
          window.alert(
            `ESP32 Board Firmware ${firmwareVersion} doesn't match with latest PianoLux ${latestReleaseTag} Firmware. Please download firmware and filesystem files to update to the latest version (Use Download and Update buttons!)`
          );
        }
      })
      .catch(error => {
        console.error("Error fetching latest release tag:", error);
        window.alert(
          "Error fetching latest release tag. Please try again later."
        );
      });
  }

  // Create the popup container
  const popup = document.createElement("div");
  popup.id = "popup";
  document.body.appendChild(popup);

  // Create the div for select elements
  const selectDiv = document.createElement("div");
  selectDiv.id = "selectDiv";
  popup.appendChild(selectDiv);

  // Create select elements
  const boardSelect = document.createElement("select");
  boardSelect.id = "boardSelect";
  boardSelect.innerHTML = `
<option value="esp32s3">ESP32-S3</option>
<option value="esp32s2">ESP32-S2</option>
<option value="esp32">ESP32</option>
`;

  const binarySelect = document.createElement("select");
  binarySelect.id = "binarySelect";
  binarySelect.innerHTML = `
<option value="firmware">Firmware</option>
<option value="filesystem">Filesystem</option>
`;

  selectDiv.appendChild(boardSelect);
  selectDiv.appendChild(binarySelect);

  // Create the div for buttons
  const buttonDiv = document.createElement("div");
  buttonDiv.id = "buttonDiv";
  popup.appendChild(buttonDiv);

  // Create buttons
  const downloadButton = document.createElement("button");
  downloadButton.id = "Download";
  downloadButton.innerText = "Download";

  const updateButton = document.createElement("button");
  updateButton.id = "Update";
  updateButton.innerText = "Update";

  const checkforUpdatesButton = document.createElement("button");
  checkforUpdatesButton.id = "checkforUpdatesButton";
  checkforUpdatesButton.innerText = "Check For Updates";

  // Create close button
  const closeButton = document.createElement("button");
  closeButton.id = "closeButton";
  closeButton.innerText = "Close";
  closeButton.onclick = function() {
    // Remove the event listener when the close button is clicked
    Socket.removeEventListener("message", handleMessage);
    document.body.removeChild(popup);
  };

  buttonDiv.appendChild(checkforUpdatesButton);
  buttonDiv.appendChild(downloadButton);
  buttonDiv.appendChild(updateButton);
  buttonDiv.appendChild(closeButton);

  checkforUpdatesButton.addEventListener("click", () => {
    console.log("Checking for updates...");

    sendData("ReadESP32Info");

    // Event listener to handle updates from the server
    Socket.addEventListener("message", handleMessage);
  });

  // Add event listeners for the "Download" and "Update" buttons
  downloadButton.addEventListener("click", () => {
    const selectedBoard = boardSelect.value;
    const selectedBinary = binarySelect.value;

    if (selectedBoard && selectedBinary) {
      fetchAssetsList(selectedBoard, selectedBinary);
    } else {
      console.error("Please select both board and binary type.");
    }
  });

  updateButton.addEventListener("click", () => {
    doOTA();
  });

  // Add CSS styles
  const style = document.createElement("style");
  style.innerHTML = `
#popup {
position: fixed;
top: 50%;
left: 50%;
transform: translate(-50%, -50%);
width: 80%; /* Set width to 80% of the screen */
max-width: 400px; /* Limit maximum width for better readability */
background-color: #ffffff;
padding: 20px;
border-radius: 12px;
box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1), 0 2px 4px rgba(0, 0, 0, 0.06);
z-index: 100; /* Keep the highest index */
}

#selectDiv select {
margin-bottom: 0px;
margin-top: 10px;
border: none;
background: #24A0ED;
border-radius: 20px;
height: auto;
font-size: 18px;
font-weight: 600;
}

#buttonDiv button {
background: #24A0ED;
border: none;
border-radius: 20px;
font-size: 18px;
font-weight: 600;
}

#closeButton {
background: #24A0ED;
border-radius: 20px;
font-size: 18px;
font-weight: 600;
}
`;

  document.head.appendChild(style);
}

// Function to fetch the latest release tag from the GitHub repository
function fetchLatestReleaseTag() {
  const githubRepoUrl =
    "https://api.github.com/repos/serifpersia/pianolux-esp32/releases/latest";

  return fetch(githubRepoUrl)
    .then(response => response.json())
    .then(data => {
      const latestReleaseTag = data.tag_name;
      return latestReleaseTag;
    });
}

function fetchAssetsList(board, fileType) {
  const githubApiUrl = `https://api.github.com/repos/serifpersia/pianolux-esp32/releases/latest`;

  fetch(githubApiUrl)
    .then(response => response.json())
    .then(data => {
      const assets = data.assets;

      if (assets.length > 0) {
        console.log("Available assets in the latest release:");
        assets.forEach(asset => {
          console.log(asset.name);
        });

        const binaryFileName = `${board}_${fileType}.bin`;
        const binaryFile = assets.find(asset => asset.name === binaryFileName);

        if (binaryFile) {
          console.log(`Fetched binary file: ${binaryFile.name}`);
          downloadFile(binaryFile.browser_download_url, binaryFile.name);
        } else {
          console.error(`Binary file ${binaryFileName} not found.`);
        }
      } else {
        console.error("No assets found in the latest release.");
      }
    })
    .catch(error => {
      console.error("Error fetching release assets:", error);
      alert("No files found on Github Repository!");
    });
}

function downloadFile(url, fileName) {
  const anchor = document.createElement("a");
  anchor.href = url;
  anchor.download = fileName;
  anchor.click();
  alert("Delete downloaded files after you upload them to ESP32 board!");
}

function doOTA() {
  window.location.href = "/update";
}

const showLogsToggleCheckbox = document.getElementById("cb6-8");
let popup; // declare popup variable outside the event listener
const maxMessages = 26; // Maximum messages to store in history
const messages = [];
let scrollTimeout; // To track the timeout for auto-scrolling

function updatePopup(message) {
  const isAtBottom = popup.scrollHeight - popup.clientHeight <= popup.scrollTop + 1;

  messages.push(message);
  const isOverLimit = messages.length > maxMessages;

  if (isOverLimit && !isAtBottom) {
    // If over limit and not at bottom, do not shift messages yet
    return;
  }

  if (isOverLimit) {
    messages.shift(); // Remove the oldest message
  }

  const popupContent = messages.map(msg => `${msg}`).join("<br>");

  // Update popup content
  popup.innerHTML = popupContent;

  if (isAtBottom) {
    // If at bottom, auto-scroll
    popup.scrollTop = popup.scrollHeight - popup.clientHeight;
  }
}

showLogsToggleCheckbox.addEventListener("click", function() {
  if (showLogsToggleCheckbox.checked) {
    console.log("The checkbox is checked: true");

    sendData("ReadESP32Logs");

    // Create the popup container
    popup = document.createElement("div");
    popup.id = "popup";
    popup.style.height = "45px"; // Set a fixed height for the popup
    popup.style.overflowY = "auto"; // Enable vertical scrolling
    document.body.appendChild(popup);

    // Add CSS styles
    const style = document.createElement("style");
    style.innerHTML = `
#popup {
  position: fixed;
  top: 10%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 80%; /* Set width to 80% of the screen */
  background-color: #000;
  padding: 15px;
  border: 2px solid gray; /* Add green border */
  border-radius: 15px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
  z-index: 10; /* Keep the highest index */
  color: #00ff00; /* Green text */
  font-family: monospace; /* Use monospace font */
}
`;
    document.head.appendChild(style);

    // Function to handle messages received from WebSocket
    function handleMessage(event) {
      const data = JSON.parse(event.data);
      const message = data.LOG_MESSAGE; // Assuming the message is contained in the LOG_MESSAGE property

      if (message !== undefined) {
        updatePopup(message); // Update popup content with received message
      }
    }

    // Event listener to handle updates from the server
    Socket.addEventListener("message", handleMessage);

    // Event listener for scroll event
    popup.addEventListener("scroll", function() {
      // Clear previous scroll timeout
      clearTimeout(scrollTimeout);

      // Set a new timeout to enable auto-scroll after 1.5 seconds
      scrollTimeout = setTimeout(function() {
        popup.scrollTop = popup.scrollHeight - popup.clientHeight;
      }, 2500);
    });

    // Create the clear button directly below the terminal
    const clearButton = document.createElement("button");
    clearButton.id = "clearButton";
    clearButton.textContent = "Clear";
    clearButton.style.position = "fixed";
    clearButton.style.width = "auto";
    clearButton.style.background = "#24A0ED";
    clearButton.style.fontSize = "15px";
    clearButton.style.fontWeight = "#300";
    clearButton.style.top = "calc(10% + 15px + 15px)"; // Adjust the top position accordingly
    clearButton.style.left = "87%";
    clearButton.style.transform = "translateX(-50%)";
    clearButton.style.zIndex = "10"; // Higher than the terminal popup
    clearButton.addEventListener("click", function() {
      messages.length = 0; // Clear the messages array
      popup.innerHTML = ""; // Clear the terminal content
    });
    document.body.appendChild(clearButton);
  } else {
    console.log("The checkbox is not checked: false");
    document.body.removeChild(popup);
    const clearButton = document.getElementById("clearButton");
    if (clearButton) {
      document.body.removeChild(clearButton);
    }
    Socket.removeEventListener("message", handleMessage);
  }
});


function showESP32Info() {
  // Send request to read ESP32 info
  sendData("ReadESP32Info");

  // Create the popup container
  const popup = document.createElement("div");
  popup.id = "popup";
  document.body.appendChild(popup);

  // Create close button
  const closeButton = document.createElement("button");
  closeButton.innerText = "Close";
  closeButton.onclick = function() {
    // Remove the event listener when the close button is clicked
    Socket.removeEventListener("message", handleMessage);
    document.body.removeChild(popup);
  };

  // Create device info element
  const deviceInfoElement = document.createElement("div");

  // Define the function to handle messages
  function handleMessage(event) {
    var data = JSON.parse(event.data);

    // Populate device info element with detailed data
    deviceInfoElement.innerHTML = `
<p>Firmware Version: PianoLux ${data.FirmwareVersion}</p>
<p>Firmware Build Date: ${data.FirmwareBuildDate}</p>
<p>Chip Model: ${data.ChipModel}</p>
<p>SSID: ${data.SSID}</p>
<p>IP Address: ${data.IPAddress}</p>
<p>MAC Address: ${data.MACAddress}</p>
<p>MDNS hostname: <a href="http://pianolux.local">pianolux.local</a></p>
<p>CPU Temperature: ${parseInt(data.Temperature).toFixed(0)} C</p>
<p>Uptime: ${data.Uptime}</p>
`;
    // Append elements to the popup
    popup.appendChild(deviceInfoElement);
    popup.appendChild(closeButton);
  }

  // Event listener to handle updates from the server
  Socket.addEventListener("message", handleMessage);

  // Add CSS styles
  const style = document.createElement("style");
  style.innerHTML = `
#popup {
position: fixed;
top: 50%;
left: 50%;
transform: translate(-50%, -50%);
width: 75%; /* Set width to 80% of the screen */
background-color: #fff;
padding: 20px;
border-radius: 10px;
box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
z-index: 10; /* Keep the highest index */
}

#popup a {
color: #007BFF; /* Blue color for the link */
text-decoration: none; /* Remove underline */
transition: color 0.3s ease; /* Smooth color transition */
}

#popup button {
background-color: #24A0ED;
color: #fff;
border: none;
border-radius: 5px;
padding: 10px 20px;
cursor: pointer;
font-size: 16px;
margin-top: 10px;
font-weight: normal;
}

#popup p {
font-size: 16px;
margin-bottom: 10px;
color: #333;
}
`;
  document.head.appendChild(style);
}

// Get the current year
var currentYear = new Date().getFullYear();

// Update the content of the footer with the current year
document.getElementById("currentYear").innerHTML += " " + currentYear;

// Call the init function when the window loads
window.onload = function(event) {
  init();
};
