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

selectedItemModes.addEventListener('click', () => {
    // Toggle the visibility of the dropdown list for LED Modes
    if (dropdownListModes.style.display === 'none' || dropdownListModes.style.display === '') {
        dropdownListModes.style.display = 'block';
    } else {
        dropdownListModes.style.display = 'none';
    }
});

// Add click event listeners to each dropdown item for LED Modes
const dropdownItemsModes = document.querySelectorAll('.dropdown-list-modes .dropdown-item');
dropdownItemsModes.forEach((item) => {
    item.addEventListener('click', () => {
        selectedItemModes.textContent = item.textContent;
        dropdownListModes.style.display = 'none'; // Compact the dropdown list for LED Modes
    });
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

var Socket;
document.getElementById('BTN_COLOR').addEventListener('click', button_changeColor);
document.getElementById('SLIDER1').addEventListener('input', slider1_changeValue);
document.getElementById('SLIDER2').addEventListener('input', slider2_changeValue);

function init() {
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
}

function button_changeColor() {
    Socket.send('ChangeColor');
}

function slider1_changeValue() {
    var value = document.getElementById('SLIDER1').value;
    Socket.send('SliderAction1:' + value);
}

function slider2_changeValue() {
    var value = document.getElementById('SLIDER2').value;
    Socket.send('SliderAction2:' + value);
}

window.onload = function(event) {
    init();
}
