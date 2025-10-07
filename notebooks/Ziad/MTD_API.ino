#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// WiFi credentials
const char* ssid = "iPhone";
const char* password = "11111111";

// MTD API endpoint
const char* apiURL = "https://developer.mtd.org/api/v2.2/json/GetVehicles?key=13d4773418a343cc902e31242d674e67";

// Pin definitions
#define PHOTORESISTOR_PIN 1
#define BUTTON1_PIN 4
#define BUTTON2_PIN 5
#define LED_PIN 36
#define NEOPIXEL_PIN 16
#define OLED_SDA_PIN 39
#define OLED_SCL_PIN 38
#define SSD1306_I2C_ADDRESS 0x3C

// LED strip setup
#define NUM_LEDS 160
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
int intensity = 255;
unsigned long lastAPIUpdate = 0;
const unsigned long API_UPDATE_INTERVAL = 60000; // 60 seconds

// Bus tracking structure
struct Bus {
  char route_id[20];
  double lat;
  double lon;
  int led_position;
  bool active;
};

Bus trackedBuses[2]; // Track 2 buses

void setup() {
  // For ESP32-S3 with USB CDC enabled
  Serial.begin(115200);
  delay(2000); // Longer delay for USB CDC to initialize
  
  // Blink onboard LED to show it's running
  pinMode(LED_BUILTIN, OUTPUT);
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  
  Serial.println("\n=== MTD Bus Tracker Initializing ===");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed - continuing without display"));
    // Don't halt, just continue without display
  } else {    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Starting...");
    display.display();
  }
  
  // Initialize NeoPixel LED strip
  strip.begin();
  strip.setBrightness(intensity);
  strip.show();
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_PIN, INPUT);
  
  // Initialize bus tracking
  for(int i = 0; i < 2; i++) {
    trackedBuses[i].active = false;
  }
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting WiFi...");
  display.display();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi Connected!");
    display.setCursor(0, 10);
    display.print(WiFi.localIP());
    display.display();
    delay(2000);
    
    // First API call
    fetchBusData();
  } else {
    Serial.println("\nWiFi Failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi Failed!");
    display.display();
  }
}

void loop() {
  // Blink LED to prove loop is running
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Loop running..."); // Debug: confirm loop is running
  
  // Update brightness from photoresistor
  int lightValue = analogRead(PHOTORESISTOR_PIN);
  intensity = map(lightValue, 0, 4095, 32, 255);
  strip.setBrightness(intensity);
  
  // Check if it's time to update from API (every 60 seconds)
  if (millis() - lastAPIUpdate > API_UPDATE_INTERVAL) {
    Serial.println("60 seconds passed, fetching data...");
    fetchBusData();
    lastAPIUpdate = millis();
  }
  
  // Read button states
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);
  
  // Serial.printf("Button states: B1=%d B2=%d\n", button1State, button2State);
  
  // Button 1: Manual refresh
  if (button1State == LOW) {
    Serial.println("Button 1: Manual refresh");
    fetchBusData();
    delay(300); // Debounce
  }
  
  // Button 2: Clear display
  if (button2State == LOW) {
    Serial.println("Button 2: Clear LEDs");
    strip.clear();
    strip.show();
    delay(300);
  }
  
  // Draw buses on LED strip
  drawBuses();
  
  // Update OLED display
  updateDisplay();
  
  delay(1000); // Changed to 1 second so Serial isn't flooded
}

void fetchBusData() {
  Serial.println("\n--- Fetching Bus Data ---");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }
  
  Serial.println("WiFi is connected, making HTTP request...");
  
  WiFiClientSecure client;
  client.setInsecure(); // Disable SSL certificate verification
  
  HTTPClient http;
  http.begin(client, apiURL);
  
  Serial.println("Sending GET request...");
  int httpCode = http.GET();
  
  Serial.printf("HTTP Response Code: %d\n", httpCode);
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.printf("API Response received, length: %d bytes\n", payload.length());
    Serial.println("First 200 chars of response:");
    Serial.println(payload.substring(0, 200));
    
    // Parse JSON (ArduinoJson v7) - allocate more memory
    JsonDocument doc;
    // Reserve capacity for large response
    if(!doc.overflowed()) {
      Serial.println("JSON document has capacity");
    }
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }
    
    Serial.println("JSON parsed successfully!");
    
    // Reset tracked buses
    for(int i = 0; i < 2; i++) {
      trackedBuses[i].active = false;
    }
    
    int busIndex = 0;
    int totalVehicles = 0;
    
    // Loop through vehicles and find TEAL and GREENHOPPER
    for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
      totalVehicles++;
      
      if (busIndex >= 2) continue; // Continue counting but only track 2 buses
      
      const char* route_id = vehicle["trip"]["route_id"];
      
      // Debug: Print every route we see
      Serial.printf("Vehicle %d: Route = %s\n", totalVehicles, route_id);
      
      // Check if this is one of our target routes
      if (strcmp(route_id, "TEAL") == 0 || strcmp(route_id, "GREENHOPPER") == 0) {
        double lat = vehicle["location"]["lat"];
        double lon = vehicle["location"]["lon"];
        
        strcpy(trackedBuses[busIndex].route_id, route_id);
        trackedBuses[busIndex].lat = lat;
        trackedBuses[busIndex].lon = lon;
        trackedBuses[busIndex].led_position = calculateLEDPosition(lat, lon);
        trackedBuses[busIndex].active = true;
        
        Serial.printf("Bus %d: %s at LED %d (%.5f, %.5f)\n", 
                      busIndex, route_id, trackedBuses[busIndex].led_position, lat, lon);
        
        busIndex++;
      }
    }
    
    Serial.printf("Total vehicles found: %d\n", totalVehicles);
    Serial.printf("Found %d target buses\n", busIndex);
    
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
  }
  
  http.end();
}

int calculateLEDPosition(double lat, double lon) {
  // Simple mapping based on latitude
  // Champaign-Urbana bounds: lat 40.09 to 40.13
  
  float minLat = 40.09;
  float maxLat = 40.13;
  
  // Constrain and map to LED range
  lat = constrain(lat, minLat, maxLat);
  int ledPos = map((int)(lat * 10000), (int)(minLat * 10000), (int)(maxLat * 10000), 0, NUM_LEDS - 1);
  
  return constrain(ledPos, 0, NUM_LEDS - 1);
}

void drawBuses() {
  // Clear strip first
  strip.clear();
  
  // Draw each active bus with trailing effect
  for(int i = 0; i < 2; i++) {
    if (trackedBuses[i].active) {
      uint32_t color;
      
      // Assign colors: TEAL = teal/cyan, GREENHOPPER = bright green
      if (strcmp(trackedBuses[i].route_id, "TEAL") == 0) {
        color = strip.Color(0, 128, 128); // Teal
      } else {
        color = strip.Color(0, 255, 0); // Bright Green
      }
      
      drawBusWithTrail(trackedBuses[i].led_position, color, 1);
    }
  }
  
  strip.show();
}

void drawBusWithTrail(int position, uint32_t color, int trailLength) {
  // Draw the bus at current position (100% brightness)
  strip.setPixelColor(position, color);
  
  // Draw trailing LEDs with decreasing brightness
  for(int i = 1; i <= trailLength; i++) {
    int trailPos = position - i;
    if (trailPos >= 0) {
      // Calculate dimmer version of color
      uint8_t r = (color >> 16) & 0xFF;
      uint8_t g = (color >> 8) & 0xFF;
      uint8_t b = color & 0xFF;
      
      float brightness = 1.0 - (i / (float)trailLength);
      
      uint32_t dimColor = strip.Color(r * brightness, g * brightness, b * brightness);
      strip.setPixelColor(trailPos, dimColor);
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  
  display.println("MTD Bus Tracker");
  display.println("---------------");
  
  for(int i = 0; i < 2; i++) {
    if (trackedBuses[i].active) {
      display.printf("%s: LED %d\n", trackedBuses[i].route_id, trackedBuses[i].led_position);
    }
  }
  
  if (!trackedBuses[0].active && !trackedBuses[1].active) {
    display.println("No buses found");
  }
  
  display.display();
}