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
#define LED_BUILTIN 48
#define NEOPIXEL_PIN 16
#define OLED_SDA_PIN 39
#define OLED_SCL_PIN 38
#define SSD1306_I2C_ADDRESS 0x3C

// LED strip setup
#define NUM_LEDS 156
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
int intensity = 125;
unsigned long lastAPIUpdate = 0;
const unsigned long API_UPDATE_INTERVAL = 60000;

// Illini Union tracking
double IU_lat = 0.0;
double IU_lon = 0.0;
bool IU_coords_found = false;
const double IU_PROXIMITY = 0.005;

// Route cycling for Button 1
const char* IU_routes[] = {"TEAL", "GREEN", "GREENHOPPER", "ILLINI", "BLUE", "YELLOW", "YELLOWHOPPER", "RED"};
const int NUM_IU_ROUTES = 8;
int currentRouteIndex = -1;
bool showingRouteColor = false;

// Bus tracking structure
struct Bus {
  char route_id[20];
  double lat;
  double lon;
  int led_position;
  bool active;
};

// Expanded buffers
Bus trackedBuses[75];
Bus nearIU_Eastbound[20];
Bus nearIU_Westbound[20];

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize LED strip first
  strip.begin();
  strip.setBrightness(125);
  strip.clear();
  strip.show();

  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  Serial.println("\n=== MTD Bus Tracker Initializing ===");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed - continuing without display"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Starting...");
    display.display();
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  for (int i = 0; i < 75; i++) trackedBuses[i].active = false;
  for (int i = 0; i < 20; i++) {
    nearIU_Eastbound[i].active = false;
    nearIU_Westbound[i].active = false;
  }

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

    fetchBusData();
    lastAPIUpdate = millis();
  } else {
    Serial.println("\nWiFi Failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi Failed!");
    display.display();
  }
}

void loop() {
  // int lightValue = analogRead(PHOTORESISTOR_PIN);
  // intensity = map(lightValue, 0, 4095, 32, 255);
  strip.setBrightness(125);

  if (millis() - lastAPIUpdate > API_UPDATE_INTERVAL) {
    Serial.println("60 seconds passed, fetching data...");
    fetchBusData();
    lastAPIUpdate = millis();
  }

  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  if (button1State == LOW) {
    currentRouteIndex++;
    if (currentRouteIndex >= NUM_IU_ROUTES) {
      currentRouteIndex = -1;
      showingRouteColor = false;
      Serial.println("Button 1: Back to normal tracking mode");
    } else {
      showingRouteColor = true;
      Serial.printf("Button 1: Showing route %s (index %d)\n", IU_routes[currentRouteIndex], currentRouteIndex);
    }
    delay(300);
  }

  if (button2State == LOW) {
    Serial.println("Button 2: Manual data refresh");
    fetchBusData();
    delay(300);
  }

  if (showingRouteColor) {
    uint32_t routeColor = getRouteColor(IU_routes[currentRouteIndex]);
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, routeColor);
    strip.show();
  } else {
    drawBuses();
  }

  updateDisplay();
  delay(1000);
}

void fetchBusData() {
  Serial.println("\n--- Fetching Bus Data ---");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, apiURL);
  int httpCode = http.GET();

  Serial.printf("HTTP Response Code: %d\n", httpCode);
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.printf("API Response received, length: %d bytes\n", payload.length());

    DynamicJsonDocument doc(65536);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    Serial.println("JSON parsed successfully!");

    if (!IU_coords_found) {
      for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
        const char* prev_stop = vehicle["previous_stop_id"];
        const char* next_stop = vehicle["next_stop_id"];
        const char* origin_stop = vehicle["origin_stop_id"];
        const char* dest_stop = vehicle["destination_stop_id"];
        if ((prev_stop && strncmp(prev_stop, "IU:", 3) == 0) ||
            (next_stop && strncmp(next_stop, "IU:", 3) == 0) ||
            (origin_stop && strncmp(origin_stop, "IU:", 3) == 0) ||
            (dest_stop && strncmp(dest_stop, "IU:", 3) == 0)) {
          IU_lat = vehicle["location"]["lat"];
          IU_lon = vehicle["location"]["lon"];
          IU_coords_found = true;
          break;
        }
      }
      if (!IU_coords_found) {
        IU_lat = 40.109;
        IU_lon = -88.227;
        IU_coords_found = true;
      }
    }

    for (int i = 0; i < 75; i++) trackedBuses[i].active = false;
    for (int i = 0; i < 20; i++) {
      nearIU_Eastbound[i].active = false;
      nearIU_Westbound[i].active = false;
    }

    int busIndex = 0, eastboundCount = 0, westboundCount = 0;

    for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
      if (busIndex >= 75) break;

      const char* route_id = vehicle["trip"]["route_id"];
      if (!route_id || strlen(route_id) == 0) continue;

      double lat = vehicle["location"]["lat"];
      double lon = vehicle["location"]["lon"];

      double lat_diff = fabs(lat - IU_lat);
      double lon_diff = fabs(lon - IU_lon);
      double distance = sqrt(lat_diff * lat_diff + lon_diff * lon_diff);

      if (distance <= IU_PROXIMITY) {
        const char* direction = vehicle["trip"]["direction"];
        bool isEastbound = false;
        if (direction) {
          String dirStr = String(direction);
          dirStr.toLowerCase();
          if (dirStr.indexOf("east") >= 0 || dirStr.indexOf("south") >= 0 || dirStr.indexOf("urbana") >= 0)
            isEastbound = true;
        }

        if (isEastbound && eastboundCount < 20) {
          strcpy(nearIU_Eastbound[eastboundCount].route_id, route_id);
          nearIU_Eastbound[eastboundCount].led_position = 10 + eastboundCount;
          nearIU_Eastbound[eastboundCount].active = true;
          eastboundCount++;
        } else if (!isEastbound && westboundCount < 20) {
          strcpy(nearIU_Westbound[westboundCount].route_id, route_id);
          nearIU_Westbound[westboundCount].led_position = 135 + westboundCount;
          nearIU_Westbound[westboundCount].active = true;
          westboundCount++;
        }
      }

      strcpy(trackedBuses[busIndex].route_id, route_id);
      trackedBuses[busIndex].lat = lat;
      trackedBuses[busIndex].lon = lon;
      trackedBuses[busIndex].led_position = map((int)(lat * 10000), 400900, 401300, 0, NUM_LEDS - 1);
      trackedBuses[busIndex].active = true;
      busIndex++;
    }

    Serial.printf("Buses near IU - Westbound: %d, Eastbound: %d\n", westboundCount, eastboundCount);
  }
  http.end();
}

void drawBuses() {
  strip.clear();

  for (int i = 0; i < 20; i++) {
    if (nearIU_Westbound[i].active)
      drawBusWithTrail(nearIU_Westbound[i].led_position, getRouteColor(nearIU_Westbound[i].route_id), 1);
  }

  for (int i = 0; i < 20; i++) {
    if (nearIU_Eastbound[i].active)
      drawBusWithTrail(nearIU_Eastbound[i].led_position, getRouteColor(nearIU_Eastbound[i].route_id), 1);
  }

  strip.show();
}

uint32_t getRouteColor(const char* route_id) {
  if (strcmp(route_id, "TEAL") == 0) return strip.Color(0, 128, 128);
  if (strcmp(route_id, "GREEN") == 0 || strcmp(route_id, "GREENHOPPER") == 0 || strcmp(route_id, "GREEN EXPRESS") == 0) return strip.Color(0, 255, 0);
  if (strcmp(route_id, "ILLINI") == 0) return strip.Color(90, 29, 90);
  if (strcmp(route_id, "YELLOW") == 0 || strcmp(route_id, "YELLOWHOPPER") == 0) return strip.Color(255, 255, 0);
  if (strcmp(route_id, "RED") == 0) return strip.Color(255, 0, 0);
  if (strcmp(route_id, "BLUE") == 0) return strip.Color(0, 0, 255);
  if (strcmp(route_id, "BROWN") == 0) return strip.Color(139, 69, 19);
  if (strcmp(route_id, "ORANGE") == 0) return strip.Color(255, 165, 0);
  if (strcmp(route_id, "PINK") == 0) return strip.Color(255, 105, 180);
  if (strcmp(route_id, "GOLD") == 0 || strcmp(route_id, "GOLDHOPPER") == 0) return strip.Color(255, 215, 0);
  if (strcmp(route_id, "GREY") == 0 || strcmp(route_id, "GRAY") == 0) return strip.Color(128, 128, 128);
  if (strcmp(route_id, "SILVER") == 0) return strip.Color(192, 192, 192);
  if (strcmp(route_id, "BRONZE") == 0) return strip.Color(205, 127, 50);
  if (strcmp(route_id, "LAVENDER") == 0) return strip.Color(230, 230, 250);
  if (strcmp(route_id, "NAVY") == 0) return strip.Color(0, 0, 128);
  if (strcmp(route_id, "RAVEN") == 0) return strip.Color(50, 50, 50);
  if (strcmp(route_id, "LINK") == 0) return strip.Color(100, 149, 237);
  return strip.Color(255, 255, 255);
}

void drawBusWithTrail(int position, uint32_t color, int trailLength) {
  if (position < 0 || position >= NUM_LEDS) return;
  strip.setPixelColor(position, color);
  int trailPos = position - 1;
  if (trailPos >= 0) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    strip.setPixelColor(trailPos, strip.Color(r * 0.25, g * 0.25, b * 0.25));
    strip.setBrightness(intensity-50);
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  if (showingRouteColor) {
    display.println("Route Display");
    display.println("---------------");
    display.printf("Route: %s\n", IU_routes[currentRouteIndex]);
    display.println("\nBtn1: Next route");
    display.println("(or back to live)");
  } else {
    display.println("IU Bus Tracker");
    display.println("---------------");
    display.printf("IU: (%.3f,%.3f)\n", IU_lat, IU_lon);
    int westCount = 0, eastCount = 0;
    for (int i = 0; i < 20; i++) {
      if (nearIU_Westbound[i].active) westCount++;
      if (nearIU_Eastbound[i].active) eastCount++;
    }
    display.printf("W/N: %d\n", westCount);
    display.printf("E/S: %d\n", eastCount);
    display.println("Btn1: Show routes");
  }

  display.display();
}