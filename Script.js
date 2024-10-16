async function toggleRelay(relayIndex) {
    const statusElement = document.getElementById('status');
    const relayStatusElement = document.getElementById('relayStatus' + relayIndex);
    const buttonElement = document.getElementById('btn' + relayIndex);

    statusElement.textContent = 'Toggling Relay ' + (relayIndex + 1) + '...';

    try {
        const response = await fetch(`/toggle/` + relayIndex);
        const result = await response.text();
        statusElement.textContent = `Relay ${relayIndex + 1} is now ${result}`;
        relayStatusElement.textContent = result;

        if (result === "ON") {
            buttonElement.classList.add("on");
        } else {
            buttonElement.classList.remove("on");
        }
    } catch (error) {
        statusElement.textContent = 'Failed to toggle relay.';
        console.error('Error:', error);
    }
}

async function loadRelayStates() {
    try {
        const response = await fetch('/states');
        const states = await response.json();

        states.forEach((state, index) => {
            const relayStatusElement = document.getElementById('relayStatus' + index);
            const buttonElement = document.getElementById('btn' + index);

            relayStatusElement.textContent = state ? 'ON' : 'OFF';
            if (state) {
                buttonElement.classList.add('on');
            } else {
                buttonElement.classList.remove('on');
            }
        });

    } catch (error) {
        console.error('Error:', error);
    }
}

async function loadTemperature() {
    const temperatureElement = document.getElementById('temperature');
    const feelsLikeElement = document.getElementById('feelsLike');

    try {
        const response = await fetch('/temperature');
        const { temperature, feelsLike } = await response.json();
        temperatureElement.textContent = `🌡 ${temperature} °C`;
        feelsLikeElement.textContent = `🌡 Heat Index ${feelsLike} °C`;
    } catch (error) {
        console.error('Error fetching temperature:', error);
        temperatureElement.textContent = '🌡 Error fetching temperature';
        feelsLikeElement.textContent = '🌡 no data';
    }
}

// Refresh relay states every 1 second
setInterval(loadRelayStates, 1000);

// Refresh temperature every 30 seconds
setInterval(loadTemperature, 30000);


document.addEventListener('DOMContentLoaded', () => {
    loadRelayStates();
    loadTemperature();
});