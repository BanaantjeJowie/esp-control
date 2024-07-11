#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define the pin numbers
const int relayPin = 2;    // GPIO 2 for the relay

// WiFi credentials
const char* ssid = "TP-Link_FA02";
const char* password = "90428562";

// Create a web server on port 80
WebServer server(80);

// Relay state
bool relayState = false;

void setup() {
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);
  
  // Start with the relay off
  digitalWrite(relayPin, LOW);
  
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

  // Define the route for toggling the relay
  server.on("/toggle", handleToggle);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Relay state: OFF");
  display.display();
}

void loop() {
  // Handle client requests
  server.handleClient();
}

void handleToggle() {
  // Toggle the relay state
  relayState = !relayState;
  digitalWrite(relayPin, relayState ? HIGH : LOW);
  Serial.print("Relay state: ");
  Serial.println(relayState ? "ON" : "OFF");
  
  // Update OLED display
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Relay state: ");
  display.print(relayState ? "ON" : "OFF");
  display.display();
  
  // Send a response to the client
  server.send(200, "text/plain", relayState ? "ON" : "OFF");
}
