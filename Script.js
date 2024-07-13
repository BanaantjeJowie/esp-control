
const espIp = 'http://192.168.129.223'; // Replace with the IP address of your ESP

async function toggleRelay(relayIndex) {
    const statusElement = document.getElementById('status');
    statusElement.textContent = 'Status: Toggling...';

    try {
        const response = await fetch(`${espIp}/toggle/${relayIndex}`);
        const result = await response.text();
        statusElement.textContent = `Status: Relay ${relayIndex} is ${result}`;
    } catch (error) {
        statusElement.textContent = 'Status: No response from esp';
        console.error('Error:', error);
    }
>>>>>>> 3e1884a1ebdc07d941d0175c86dd16b3c4e82304
}
