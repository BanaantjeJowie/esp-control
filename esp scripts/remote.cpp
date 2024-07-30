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
const int charWidth = 6; // Approximate width of a character in pixels

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
  tft.setTextColor(ST77XX_WHITE);

  // Clear previous indicator
  if (previousOption >= 0) {
    int previousOptionWidth = strlen(options[previousOption]) * charWidth;
    tft.fillRect(10 + previousOptionWidth + 5, 10 + 20 * previousOption, 10, 10, ST77XX_BLACK);
  }

  // Draw new indicator
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

  // Clear the screen
  tft.fillScreen(ST77XX_BLACK);

  // Initial display of options
  for (int i = 0; i < numOptions; i++) {
    tft.setCursor(10, 10 + 20 * i);
    tft.print(options[i]);
  }

  // Display initial selection
  displayOptions();

  // Setup buttons
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleSelectButton, CHANGE);

  pinMode(SCROLL_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SCROLL_BUTTON_PIN), handleScrollButton, CHANGE);

  // Connect to WiFi
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
}
