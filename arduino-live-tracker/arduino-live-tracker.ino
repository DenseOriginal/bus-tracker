#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <PinButton.h>
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

// --- Configuration ---
const unsigned long STAY_AWAKE_TIME = 1000 * 60 * 5; // 5 minutes in milliseconds
const unsigned long REFRESH_INTERVAL = 1000 * 30; // Refresh data every 30s while awake
const unsigned int MAX_DEPARTURE_DELTA = 25;

#define BEEP_TIME 400
#define BEEP_FREQUENCY 1200

#define ALARM_BLINK_TIME 5000

#define BASE_BRIGHTNESS 0.2

// --- Location configuration ---
const unsigned int VEJLBY = 0;
const unsigned int SKEJBY = 1;

// --- Global Variables ---
PinButton button(BUTTON_PIN);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int vejlbyDelta = 0;
int skejbyDelta = 0;

bool isAwake = true;               // Track current state
unsigned long lastUpdateTime = 0;  // When did we last fetch data?
unsigned long goToSleepTime = 0;   // When should we go to sleep?
bool dataUpdated = false;

unsigned long alarmTime; // Track when to to beep

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
  button.update();
  
  if (button.isSingleClick()) {
    if (!isAwake) return exitSleepMode();
  }

  // 2. IF AWAKE, HANDLE TASKS
  if (isAwake) {
    unsigned long currentMillis = millis();
    runBeep();
    showFlashingAlarm();
    checkAlarm();

    if (button.isLongClick()) {
      toggleAlarm();
    }

    // Check if 5 minutes have passed
    if (currentMillis > goToSleepTime) {
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
      setLedColor(SKEJBY, skejbyDelta);
      setLedColor(VEJLBY, vejlbyDelta);
      strip.show();

      dataUpdated = false;
    }

  }
}

void startupAnimation() {
  unsigned long diff = unsigned (goToSleepTime - millis());
  double brightness = ((sin(diff / 100) + 1) / 2) / 2;
  strip.fill(strip.Color(20 * brightness, 20 * brightness, 200 * brightness));
  strip.show();
}

uint32_t getLedColor(int minutes, double brightness = 1) {
  if (minutes > MAX_DEPARTURE_DELTA || minutes < 0) {
    // Too far away, turn off
    return strip.Color(0, 0, 0);
  } else {
    // Map time to color: 
    // Closer to 0 -> More Green, Less Red
    // Closer to 25 -> Less Green, More Red
    int redVal = min(map(minutes, 0, 10, 0, 255), (long) 255);
    int greenVal = map(minutes, 0, 25, 255, 0);
    
    // Set color (Red, Green, Blue)
    return strip.Color(redVal * brightness * BASE_BRIGHTNESS, greenVal * brightness * BASE_BRIGHTNESS, 0);
  }
}

// Helper: Calculate Color based on minutes
// 0 mins = Green, 30 mins = Red, >30 mins = Off
void setLedColor(int pixelIndex, int minutes) {
  strip.setPixelColor(pixelIndex, getLedColor(minutes, 1));
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
  WiFi.disconnect();
  WiFi.forceSleepBegin();
  delay(1);

  isAwake = false;
}

// Turn on WiFi, LCD, and reset timers
void exitSleepMode() {
  Serial.println("Waking Up...");

  // 2. Turn on WiFi
  WiFi.forceSleepWake();
  delay(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // 3. Reset Timers
  goToSleepTime = millis() + STAY_AWAKE_TIME;
  
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

void toggleAlarm() {
  if (alarmTime > 0) {
    stopAlarm();    
  } else {
    setAlarm();
  }
}

int ledToBlink = -1;
void setAlarm() {
  // Ignore if no bus is close enough
  if (vejlbyDelta > MAX_DEPARTURE_DELTA && skejbyDelta > MAX_DEPARTURE_DELTA) return;

  unsigned int soonestBus = (unsigned int) max(min(skejbyDelta, vejlbyDelta), 0);
  alarmTime = millis() + ((soonestBus - 1) * 1000 * 60);
  goToSleepTime = alarmTime + (1000 * 60);

  ledToBlink = skejbyDelta < vejlbyDelta ? SKEJBY : VEJLBY;

  Serial.print("Setting alarm for "); Serial.print(soonestBus - 1); Serial.println(" minutes");
}

void showFlashingAlarm() {
  if (ledToBlink != -1) {
    unsigned int diff = millis();
    double brightness = ((sin(diff / 200) + 1) / 2) * 0.6 + 0.2;
    int minutes = ledToBlink == VEJLBY ? vejlbyDelta : skejbyDelta;
    strip.setPixelColor(ledToBlink, getLedColor(minutes, brightness));

    strip.show();
  }
}

void checkAlarm() {
  if (alarmTime > 0 && millis() > alarmTime) {
    beep();
    stopAlarm();
  }
}

void stopAlarm() {
    alarmTime = 0;
    ledToBlink = -1;

    // Reset LEDs
    dataUpdated = true;
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