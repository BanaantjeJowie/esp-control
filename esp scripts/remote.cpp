#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

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
#define USB_DETECT_PIN 36     // GPIO pin for detecting USB power, adjust as needed

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
  "Option 1",
  "Option 2",
  "Option 3 (3)",
  "Option 4 (4)",
  "Beep (5)"
};
const int numOptions = sizeof(options) / sizeof(options[0]);
int selectedOption = 0;
int previousOption = -1;
bool optionStates[numOptions] = {false};  // Track the state of each option
const int charWidth = 6; // Approximate width of a character in pixels

// Battery voltage parameters
const float maxBatteryVoltage = 4.2;  // Maximum voltage when battery is fully charged
const float minBatteryVoltage = 3.0;  // Minimum voltage considered empty
const int analogMax = 4095;           // Maximum ADC value (12-bit ADC)
const float referenceVoltage = 3.3;   // Reference voltage of ADC
unsigned long lastBatteryCheck = 0;   // Timer for battery level check

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

  for (int i = 0; i < numOptions; i++) {
    if (optionStates[i]) {
      tft.setTextColor(ST77XX_GREEN);
    } else {
      tft.setTextColor(ST77XX_WHITE);
    }

    if (i == previousOption) {
      tft.fillRect(10 + strlen(options[i]) * charWidth + 5, 10 + 20 * i, 10, 10, ST77XX_BLACK);
    }

    tft.setCursor(10, 10 + 20 * i);
    tft.print(options[i]);
  }

  int selectedOptionWidth = strlen(options[selectedOption]) * charWidth;
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10 + selectedOptionWidth + 5, 10 + 20 * selectedOption);
  tft.print("<");

  previousOption = selectedOption;
}

// Function to send an API request based on the selected option
void sendAPIRequest(int option) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://192.168.129.223/toggle/" + String(option);
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
  bool usbConnected = digitalRead(USB_DETECT_PIN) == HIGH;

  if (usbConnected) {
    tft.fillRect(0, tft.height() - 20, tft.width(), 20, ST77XX_BLACK);  // Clear previous display
    tft.setCursor(10, tft.height() - 20);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("USB Power Connected");
  } else {
    int analogValue = analogRead(BATTERY_PIN);
    float voltage = analogValue * (referenceVoltage / analogMax) * 2;  // Voltage factor adjustment
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

  pinMode(USB_DETECT_PIN, INPUT);  // Set USB detection pin mode

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
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
}
