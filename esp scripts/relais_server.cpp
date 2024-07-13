#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

// Define the pin numbers
const int relayPins[] = {2, 4, 5, 16, 17, 18};    // GPIOs for the relays
const int numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

// WiFi credentials
const char* ssid = "TP-Link_FA02";
const char* password = "90428562";

// Create a web server on port 80
WebServer server(80);

// Relay states
bool relayStates[numRelays] = {false};

void setup() {
  // Initialize the relay pins as outputs
  for (int i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define the routes for toggling each relay
  for (int i = 0; i < numRelays; i++) {
    server.on(String("/toggle/") + i, [i]() { handleToggle(i); });
  }

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}

void handleToggle(int relayIndex) {
  // Toggle the relay state
  relayStates[relayIndex] = !relayStates[relayIndex];
  digitalWrite(relayPins[relayIndex], relayStates[relayIndex] ? HIGH : LOW);
  Serial.print("Relay ");
  Serial.print(relayIndex);
  Serial.print(" state: ");
  Serial.println(relayStates[relayIndex] ? "ON" : "OFF");

  // Send a response to the client
  server.send(200, "text/plain", relayStates[relayIndex] ? "ON" : "OFF");
}
