let port;

async function connectToDevice() {
    try {
        // Request a serial port
        port = await navigator.serial.requestPort();

        // Open the serial port
        await port.open({ baudRate: 115200 });

        // Send the command (byte 255)
        const writer = port.writable.getWriter();
        const command = new Uint8Array([255]);
        await writer.write(command);
        await writer.releaseLock();

        // Listen for the response
        const reader = port.readable.getReader();
        let receivedData = '';
        while (true) {
            const result = await reader.read();
            if (result.done) {
                break;
            }
            const chunk = new TextDecoder().decode(result.value);
            receivedData += chunk;

            // Check for termination character or specific length of data
            if (chunk.includes('\n')) {
                // Assuming '\n' indicates the end of the IP address
                break;
            }
        }

        // Display the complete IP
        console.log("Received IP:", receivedData.trim());

        // Show the popup with device information
        showPopup("ESP32 Device IP: " + receivedData.trim());
    } catch (error) {
        console.error("Error:", error);
    }
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

function openLink() {
    // Get the IP address from the deviceInfo element
    const deviceInfoElement = document.getElementById('deviceInfo');
    const ipAddress = deviceInfoElement.innerText.replace('ESP32 Device IP: ', '');

    // Set the current page's URL to the desired link
    window.location.href = 'http://' + ipAddress;
}

// Fetch README content from GitHub and render it
async function fetchAndRenderReadmeContent() {
    try {
        const response = await fetch('https://api.github.com/repos/serifpersia/pianoled-esp32/readme');
        const data = await response.json();
        const markdownContent = atob(data.content); // GitHub API returns base64-encoded content

        // Convert Markdown to HTML using GitHub Markdown API
        const htmlContent = await convertMarkdownToHTML(markdownContent);

        // Update the div with the rendered HTML content
        const readmeDiv = document.getElementById('readme-content');
        readmeDiv.innerHTML = htmlContent;
    } catch (error) {
        console.error("Error:", error);
    }
}

// Convert Markdown to HTML using GitHub Markdown API
async function convertMarkdownToHTML(markdownContent) {
    const response = await fetch('https://api.github.com/markdown', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            text: markdownContent,
            mode: 'gfm', // GitHub Flavored Markdown
        }),
    });

    return response.text();
}

// Call the function to fetch and render the README content when the page loads
window.onload = fetchAndRenderReadmeContent;