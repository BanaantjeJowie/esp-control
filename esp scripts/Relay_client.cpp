#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pin numbers
const int buttonPin = D4;  // GPIO D4 for the pushbutton
const int SDA_PIN = D5;    // GPIO D5 (SDA)
const int SCL_PIN = D6;    // GPIO D6 (SCL)

// WiFi credentials
const char* ssid = "TP-Link_FA02";
const char* password = "90428562";

// Server address (ESP32 IP address)
const char* serverAddress = "192.168.129.223"; // Replace with the actual IP address

// Variables to store button states
int buttonState = 0;       // Current state of the button
int lastButtonState = 0;   // Previous state of the button

// Variables for debouncing
unsigned long lastDebounceTime = 0;  // The last time the output pin was toggled
unsigned long debounceDelay = 50;    // The debounce time; increase if the output flickers

// OLED display configuration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables to store relay status and Wi-Fi status
bool relayStatus = false;
String wifiStatus = "Connecting...";

void setup() {
  // Initialize the button pin as an input with an internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
  
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Initialize I2C communication with the specified SDA and SCL pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    wifiStatus = "Connecting...";
    updateDisplay();
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  wifiStatus = "Connected: " + WiFi.localIP().toString();
  updateDisplay();
}

void loop() {
  // Read the state of the pushbutton
  int reading = digitalRead(buttonPin);

  // If the button state has changed (due to noise or pressing)
  if (reading != lastButtonState) {
    // Reset the debouncing timer
    lastDebounceTime = millis();
  }

  // If the time since the last state change is greater than the debounce delay
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // Only send the request if the new button state is LOW (button pressed)
      if (buttonState == LOW) { // Note: LOW because of the internal pull-up
        toggleRelay();
      }
    }
  }

  // Save the reading. Next time through the loop, it'll be the lastButtonState
  lastButtonState = reading;

  // Update display periodically
  updateDisplay();
}

void toggleRelay() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    String url = String("http://") + serverAddress + "/toggle";
    
    http.begin(client, url);  // Use the new API with WiFiClient
    
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Response: " + payload);
      relayStatus = (payload == "ON");
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
    wifiStatus = "Disconnected";
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  // Display Wi-Fi status
  display.setTextSize(1); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.println(wifiStatus);

  // Display relay status
  display.setCursor(0, 20);
  display.print("Relay: ");
  display.println(relayStatus ? "ON" : "OFF");

  display.display();
}
