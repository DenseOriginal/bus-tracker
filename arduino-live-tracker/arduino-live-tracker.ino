#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "./conf.h"

// --- Configuration ---
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const String serverUrl = BUS_API_URL;

// --- Pin Definitions ---
#define LED_PIN     D3
#define BUTTON_PIN  D2  // Button to wake up
#define BUZZER_PIN  D1
#define LED_COUNT   2

// --- Timing Configuration ---
const unsigned long STAY_AWAKE_TIME = 1000 * 60 * 5; // 5 minutes in milliseconds
const unsigned long REFRESH_INTERVAL = 1000 * 30; // Refresh data every 30s while awake

#define BEEP_TIME 400
#define BEEP_FREQUENCY 1200

// --- Global Variables ---
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int vejlbyDelta = 0;
int skejbyDelta = 0;

bool isAwake = true;               // Track current state
unsigned long wakeStartTime = 0;   // When did we wake up?
unsigned long lastUpdateTime = 0;  // When did we last fetch data?
bool dataUpdated = false;

// Volatile variable for Interrupts (must be volatile)
bool buttonState = false;

// --- Main Setup ---
void setup() {
  Serial.begin(115200);

  Serial.print("API_URL: "); Serial.println(serverUrl);

  // Hardware Init
  strip.begin();
  strip.show();

  // Button Init with Pullup
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Start in Awake mode
  exitSleepMode();
}

// --- Main Loop ---
void loop() {
  unsigned long currentMillis = millis();

  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (buttonState != currentButtonState && currentButtonState == 0) {
    exitSleepMode(); 
  }
  buttonState = currentButtonState;

  // 2. IF AWAKE, HANDLE TASKS
  if (isAwake) {
    runBeep();

    // Check if 5 minutes have passed
    if (
      currentMillis > wakeStartTime &&
      (currentMillis - wakeStartTime) >= STAY_AWAKE_TIME
    ) {
      enterSleepMode();
    } else {
      // If we are still within the 5 minutes, check if we need to refresh data
      // (Refresh every 30 seconds)
      // Note: lastUpdateTime is set to 0 on wake, so this runs immediately on wake
      if (currentMillis - lastUpdateTime >= REFRESH_INTERVAL || lastUpdateTime == 0) {
        
        // Wait for WiFi connection before updating
        if(WiFi.status() == WL_CONNECTED) {
           fetchData();
           lastUpdateTime = currentMillis;
        } 
        else {
           // Simple feedback if waiting for WiFi
           startupAnimation();
          //  lcd.setCursor(0,1);
          //  lcd.print(".");
          //  delay(500); // Small delay just to not flood serial while connecting
        }
      }
    }

    if (dataUpdated) {
      setLedColor(1, skejbyDelta);
      setLedColor(0, vejlbyDelta);
      strip.show();

      beep();

      dataUpdated = false;
    }

  }
}

void startupAnimation() {
  unsigned long diff = millis() - wakeStartTime;
  double brightness = ((sin(diff / 100) + 1) / 2) / 2;
  strip.fill(strip.Color(20 * brightness, 20 * brightness, 200 * brightness));
  strip.show();
}

// Helper: Calculate Color based on minutes
// 0 mins = Green, 30 mins = Red, >30 mins = Off
void setLedColor(int pixelIndex, int minutes) {
  if (minutes > 25 || minutes < 0) {
    // Too far away, turn off
    strip.setPixelColor(pixelIndex, strip.Color(0, 0, 0));
  } else {
    // Map time to color: 
    // Closer to 0 -> More Green, Less Red
    // Closer to 25 -> Less Green, More Red
    int redVal = min(map(minutes, 0, 10, 0, 255), (long) 255);
    int greenVal = map(minutes, 0, 25, 255, 0);
    
    // Set color (Red, Green, Blue)
    strip.setPixelColor(pixelIndex, strip.Color(redVal * 0.2, greenVal * 0.2, 0));
  }
}

// Turn off WiFi, LCD, and LEDs
void enterSleepMode() {
  if (!isAwake) return; // Already asleep

  Serial.println("Entering Sleep Mode...");
  
  // 1. Turn off LEDs
  strip.clear();
  strip.show();

  // 3. Turn off WiFi to save power
  WiFi.mode(WIFI_OFF); 

  isAwake = false;
}

// Turn on WiFi, LCD, and reset timers
void exitSleepMode() {
  Serial.println("Waking Up...");

  // 2. Turn on WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // 3. Reset Timers
  wakeStartTime = millis();
  
  // Force an immediate update in the main loop
  lastUpdateTime = 0; 
  isAwake = true;
}

void fetchData() {
  // Ensure WiFi is connected before requesting
  if (WiFi.status() != WL_CONNECTED) {
    // Attempt to reconnect if lost
    // Note: Standard WiFi.begin is non-blocking usually, 
    // but we need it connected to fetch.
    return; 
  }

  WiFiClientSecure client;
  client.setInsecure(); // Ignore SSL certificate validation for simplicity
  HTTPClient http;

  Serial.print("Fetching data...");
  http.begin(client, serverUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    DynamicJsonDocument doc(2048); 
    DeserializationError error = deserializeJson(doc, http.getStream());

    if (!error) {
      skejbyDelta = doc["skejby"] ? doc["skejby"]["leaveIn"] : -1;
      vejlbyDelta = doc["vejlby"] ? doc["vejlby"]["leaveIn"] : -1;
      dataUpdated = true;
      Serial.println(" Done.");
    } else {
      // lcd.clear(); lcd.print("JSON Error");
    }
  } else {
    // lcd.clear(); lcd.print("Err: " + String(httpCode));
  }
  http.end();
}

unsigned long beepStart = 0;
void beep() {
  beepStart = millis();
}

void runBeep() {
  if (beepStart == 0) return;

  if (beepStart + BEEP_TIME < millis()) {
    beepStart = 0;
    noTone(BUZZER_PIN);
    return;
  }

  unsigned int beepDuration = millis() - beepStart;
  if (int (beepDuration / (BEEP_TIME / 5)) % 2 == 0) {
    tone(BUZZER_PIN, BEEP_FREQUENCY);
  } else {
    noTone(BUZZER_PIN);
  }
}