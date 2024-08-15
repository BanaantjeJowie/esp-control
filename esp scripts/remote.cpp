#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Pin configurations
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4
#define BUTTON_PIN 0          // Select button
#define SCROLL_BUTTON_PIN 35  // Scroll button
#define BATTERY_PIN 34        // Analog pin for battery voltage

// WiFi credentials
const char* ssid = "TP-Link_FA02";
const char* password = "90428562";

// TFT display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Button states and debounce
volatile bool selectButtonPressed = false;
volatile bool scrollButtonPressed = false;
volatile bool selectButtonState = LOW;
volatile bool scrollButtonState = LOW;
volatile bool lastSelectButtonState = HIGH;
volatile bool lastScrollButtonState = HIGH;
unsigned long lastDebounceTimeSelect = 0;
unsigned long lastDebounceTimeScroll = 0;
const unsigned long debounceDelay = 50; // Adjust as needed

// Options for toggle modes
const char* options[] = {
  "Toggle Light",
  "Led Strip",
  "Option 2",
  "Option 3",
  "Beep",
  "Option 5"
};
const int numOptions = sizeof(options) / sizeof(options[0]);
int selectedOption = 0;
int previousOption = -1;
bool optionStates[numOptions] = {false};  // Track the state of each option
const int charWidth = 6; // Approximate width of a character in pixels

// Threshold to infer USB power based on voltage
const float usbPowerThreshold = 4.0;  // Adjust this value based on your battery and charging characteristics


// Battery voltage parameters
const float maxBatteryVoltage = 4.2;  // Maximum voltage when battery is fully charged
const float minBatteryVoltage = 3.0;  // Minimum voltage considered empty
const int analogMax = 4095;           // Maximum ADC value (12-bit ADC)
const float referenceVoltage = 3.3;   // Reference voltage of ADC
unsigned long lastBatteryCheck = 0;   // Timer for battery level check

// Timer for updating time and WiFi status
unsigned long lastTimeUpdate = 0;     // Timer for time update
unsigned long lastWiFiStatusUpdate = 0; // Timer for WiFi status update
const unsigned long updateInterval = 30000; // Update time and WiFi status every 30 seconds

// ISR for button presses
void IRAM_ATTR handleSelectButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTimeSelect > debounceDelay) {
    lastDebounceTimeSelect = currentTime;
    selectButtonState = digitalRead(BUTTON_PIN);
    if (selectButtonState == LOW && lastSelectButtonState == HIGH) {
      selectButtonPressed = true;
    }
    lastSelectButtonState = selectButtonState;
  }
}

void IRAM_ATTR handleScrollButton() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTimeScroll > debounceDelay) {
    lastDebounceTimeScroll = currentTime;
    scrollButtonState = digitalRead(SCROLL_BUTTON_PIN);
    if (scrollButtonState == LOW && lastScrollButtonState == HIGH) {
      scrollButtonPressed = true;
    }
    lastScrollButtonState = scrollButtonState;
  }
}

// Function to display the current options
void displayOptions() {
  tft.setTextSize(1);
  int startY = 30; // Starting Y position for the options list to avoid overlap with WiFi icon

  for (int i = 0; i < numOptions; i++) {
    if (optionStates[i]) {
      tft.setTextColor(ST77XX_GREEN);
    } else {
      tft.setTextColor(ST77XX_WHITE);
    }

    if (i == previousOption) {
      tft.fillRect(10 + strlen(options[i]) * charWidth + 5, startY + 20 * i, 10, 10, ST77XX_BLACK);
    }

    tft.setCursor(10, startY + 20 * i);
    tft.print(options[i]);
  }

  int selectedOptionWidth = strlen(options[selectedOption]) * charWidth;
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10 + selectedOptionWidth + 5, startY + 20 * selectedOption);
  tft.print("<");

  previousOption = selectedOption;
}

// Function to send an API request based on the selected option
void sendAPIRequest(int option) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://192.168.129.2/toggle/" + String(option);
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);

      if (response == "ON") {
        optionStates[option] = true;
      } else if (response == "OFF") {
        optionStates[option] = false;
      }

      displayOptions();
    } else {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

// Function to measure battery voltage and display battery level
void displayBatteryLevel() {
  int analogValue = analogRead(BATTERY_PIN);
  float voltage = analogValue * (referenceVoltage / analogMax) * 2;  // Voltage factor adjustment
  bool usbConnected = (voltage >= usbPowerThreshold);

  if (usbConnected) {
    tft.fillRect(0, tft.height() - 20, tft.width(), 20, ST77XX_BLACK);  // Clear previous display
    tft.setCursor(10, tft.height() - 20);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("USB Power Connected");
  } else {
    float batteryPercentage = (voltage - minBatteryVoltage) / (maxBatteryVoltage - minBatteryVoltage) * 100;

    if (batteryPercentage > 100) {
      batteryPercentage = 100;
    } else if (batteryPercentage < 0) {
      batteryPercentage = 0;
    }

    tft.fillRect(0, tft.height() - 20, tft.width(), 20, ST77XX_BLACK);  // Clear previous display
    tft.setCursor(10, tft.height() - 20);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Battery: ");
    tft.print(batteryPercentage, 1);  // Display percentage with one decimal point
    tft.print("%");
  }
}

// Function to display WiFi signal strength using bars
void displayWiFiStatus() {
  int16_t x = tft.width() - 20;  // Start position for WiFi icon on the far right
  int16_t y = 0;  // Vertical position for WiFi icon
  int16_t dotRadius = 2;
  int16_t barWidth = 2;
  int16_t barSpacing = 4;
  int16_t maxBarHeight = 10;

  tft.fillRect(x, y, tft.width() / 2, 20, ST77XX_BLACK);  // Clear previous WiFi display

  if (WiFi.status() == WL_CONNECTED) {
    int32_t rssi = WiFi.RSSI();

    int strength = 0;
    if (rssi >= -50) strength = 4;  // Excellent
    else if (rssi >= -60) strength = 3;  // Good
    else if (rssi >= -70) strength = 2;  // Fair
    else if (rssi < -70) strength = 1;  // Weak

    // Draw the bars
    for (int i = 0; i < strength; i++) {
      int barHeight = maxBarHeight / 4 * (i + 1);
      tft.fillRect(x + barSpacing * (i + 1), y + (maxBarHeight - barHeight), barWidth, barHeight, ST77XX_WHITE);
    }
  } else {
    tft.setCursor(x + 10, y + 4);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.print("No WiFi");
  }
}

// Function to fetch and display time
void displayTime() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://timeapi.io/api/Time/current/zone?timeZone=Europe/Brussels");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        const char* time = doc["time"].as<const char*>();

        // Display the time
        tft.fillRect(0, 0, tft.width() / 2, 20, ST77XX_BLACK);  // Clear previous time display
        tft.setCursor(0, 0);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(1);
        tft.print(time);
      } else {
        Serial.print("Error parsing JSON: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("Error on sending GET request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Hello! ST77xx TFT Test"));

  // Initialize the TFT display
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.init(135, 240);
  tft.setRotation(2);

  tft.fillScreen(ST77XX_BLACK);

  displayOptions();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleSelectButton, CHANGE);

  pinMode(SCROLL_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SCROLL_BUTTON_PIN), handleScrollButton, CHANGE);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  displayTime(); // Initial time display
  displayWiFiStatus(); // Initial WiFi status display
  displayBatteryLevel(); // Initialize battery level display
}

void loop() {
  if (scrollButtonPressed) {
    scrollButtonPressed = false;
    selectedOption = (selectedOption + 1) % numOptions;
    displayOptions();
  }

  if (selectButtonPressed) {
    selectButtonPressed = false;
    sendAPIRequest(selectedOption);
  }

  unsigned long currentTime = millis();
  if (currentTime - lastBatteryCheck > 5000) {  // Refresh battery status every 5 seconds
    lastBatteryCheck = currentTime;
    displayBatteryLevel();
  }

  if (currentTime - lastTimeUpdate > updateInterval) {  // Refresh time every 30 seconds
    lastTimeUpdate = currentTime;
    displayTime();
  }

  if (currentTime - lastWiFiStatusUpdate > updateInterval) {  // Refresh WiFi status every 30 seconds
    lastWiFiStatusUpdate = currentTime;
    displayWiFiStatus();
  }
}
