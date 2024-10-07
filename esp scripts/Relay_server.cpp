#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Preferences.h>
#include <DHT.h>

// Define the pin numbers
const int relayPins[] = {2, 4, 5, 16, 17, 18};    // GPIOs for the relays
const int buttonPin = 15;                         // GPIO for the pushbutton
const int numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

// DHT11 Sensor Setup
#define DHTPIN 19  // Define the pin where the DHT11 is connected
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;

// WiFi credentials
const char* ssid = "TP-Link_FA02";
const char* password = "90428562";

// Static IP address
IPAddress localIP(192, 168, 129, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Create a web server on port 80
WebServer server(80);

// Relay states
bool relayStates[numRelays] = {false};

// Preferences to store relay states
Preferences preferences;

// Debounce variables for pushbutton
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = LOW;
int buttonState = HIGH;  // Assume the button is not pressed

// Function to handle toggle requests
void handleToggle(int relayIndex) {
  relayStates[relayIndex] = !relayStates[relayIndex];
  digitalWrite(relayPins[relayIndex], relayStates[relayIndex] ? HIGH : LOW);
  preferences.putBool(String(relayIndex).c_str(), relayStates[relayIndex]);

  Serial.print("Relay ");
  Serial.print(relayIndex);
  Serial.print(" state: ");
  Serial.println(relayStates[relayIndex] ? "ON" : "OFF");
  server.send(200, "text/plain", relayStates[relayIndex] ? "ON" : "OFF");
}

// HTML, CSS, and JS content (with added temperature display)
const char* htmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Room Control</title>
    <style>
         body {
  font-family: Arial, sans-serif;
  background-color: #3a3a3a;
  display: flex;
  justify-content: center;
  align-items: center;
  flex-direction: column; /* Add this line to make flex children stack vertically */
  height: 100vh;
  margin: 0;
  background-size: cover;
  background-position: center;
}
.title {
  background-color: #ffffff;
  padding: 10px;
  border-radius: 10px;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  width: 350px;
  text-align: center;
  position: relative;
  margin-bottom: 20px;
}
.h3 {
  color: #333;
  margin-top: 5px;
}
.container {
  background-color: #ffffff;
  padding: 20px;
  border-radius: 10px;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  width: 350px;
  text-align: center;
  position: relative;
}
.container2 {
  background-color: #ffffff;
  padding: 20px;
  border-radius: 10px;
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  width: 350px;
  text-align: left;
  position: relative;
  margin-top: 50px;
}
h1 {
  color: #333;
  margin-bottom: 10px;
}
.grid-container {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
}
.button {
  background-color: #888;
  color: white;
  border: none;
  border-radius: 10px;
  padding: 15px;
  font-size: 16px;
  cursor: pointer;
  width: 100%;
  transition: background-color 0.3s, transform 0.3s;
}
.button.on {
  background-color: #007bff;
  transform: scale(1.05);
}
.button:hover {
  background-color: #555;
}
.relay-status {
  margin-top: 5px;
  font-size: 14px;
  color: #333;
}
.link {
  margin-top: 20px;
  display: block;
  color: #007bff;
  text-decoration: none;
  font-size: 16px;
}
.link:hover {
  text-decoration: underline;
}
.temperature {
  position: relative;
  top: 10px;
  font-size: 18px;
  color: #333;
  padding: 15px;
}
    </style>
</head>
<body>
    <div class="title">
        <h3 class="h3">BanaantjeJowie's Room</h3>
    </div>
    <div class="container">
        <h3 style="text-align: left;">Lights</h3>
        <div class="grid-container">
            <div>
                <button id="btn0" class="button" onclick="toggleRelay(0)">Main</button>
                <p class="relay-status" id="relayStatus0">OFF</p>
            </div>
            <div>
                <button id="btn1" class="button" onclick="toggleRelay(1)">Bed LED's</button>
                <p class="relay-status" id="relayStatus1">OFF</p>
            </div>
            <div>
                <button id="btn2" class="button" onclick="toggleRelay(2)">Unused</button>
                <p class="relay-status" id="relayStatus2">OFF</p>
            </div>
            <div>
                <button id="btn3" class="button" onclick="toggleRelay(3)">Unused</button>
                <p class="relay-status" id="relayStatus3">OFF</p>
            </div>
            <div>
                <button id="btn4" class="button" onclick="toggleRelay(4)">Beep</button>
                <p class="relay-status" id="relayStatus4">OFF</p>
            </div>
            <div>
                <button id="btn5" class="button" onclick="toggleRelay(5)">Unused</button>
                <p class="relay-status" id="relayStatus5">OFF</p>
            </div>
        </div>
        
    </div>
    <div class="container2">
        <h3>Statistics</h3>
        <div class="temperature" id="temperature">Temp: -- °C</div>
    </div>
    <script>
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

            try {
                const response = await fetch('/temperature');
                const temperature = await response.json();
                temperatureElement.textContent = `Temp: ${temperature} °C`;
            } catch (error) {
                console.error('Error fetching temperature:', error);
                temperatureElement.textContent = 'Temp: -- °C';
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
    </script>
</body>
</html>
)rawliteral";


void setup() {
  // Initialize preferences
  preferences.begin("relayStates", false);

  // Initialize DHT11
  dht.begin();

  // Set up relays and load their states
  for (int i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    relayStates[i] = preferences.getBool(String(i).c_str(), false);
    digitalWrite(relayPins[i], relayStates[i] ? HIGH : LOW);
  }

  // Set up pushbutton
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);

  // Configure static IP
  WiFi.config(localIP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", htmlContent); 
  });

  // Endpoint to return current relay states as JSON
  server.on("/states", []() {
    String states = "[";
    for (int i = 0; i < numRelays; i++) {
      states += relayStates[i] ? "true" : "false";
      if (i < numRelays - 1) states += ",";
    }
    states += "]";
    server.send(200, "application/json", states);
  });

  // Endpoint to return temperature as JSON
  server.on("/temperature", []() {
    temperature = dht.readTemperature();
    if (isnan(temperature)) {
      Serial.println("Failed to read temperature from DHT sensor!");
      server.send(200, "application/json", "null");
    } else {
      server.send(200, "application/json", String(temperature));
    }
  });

  for (int i = 0; i < numRelays; i++) {
  int relayIndex = i;  // Capture i by value
  server.on(String("/toggle/") + i, [relayIndex]() { handleToggle(relayIndex); });
}

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();

  // Read the pushbutton state
  int reading = digitalRead(buttonPin);

  // Check for button press and debounce
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && buttonState == HIGH) {
      handleToggle(0);  // Toggle the first relay (lights)
    }
    buttonState = reading;
  }

  lastButtonState = reading;
}
