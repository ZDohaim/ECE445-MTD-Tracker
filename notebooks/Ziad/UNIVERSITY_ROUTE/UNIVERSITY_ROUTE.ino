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

// Timing constants
#define API_UPDATE_INTERVAL 60000   
#define BUS_MOVEMENT_INTERVAL 30000  

// Geographic bounds for the tracked route segment
// White Street coordinates
#define WHITE_STREET_START_LON -88.237091  // White & 2nd
#define WHITE_STREET_END_LON -88.230445    // White & Wright (turn)
#define WHITE_STREET_LAT 40.114400         // Approximate latitude for White Street

// Wright Street coordinates
#define WRIGHT_TURN_LAT 40.114424          // White & Wright (turn)
#define WRIGHT_END_LAT 40.111529           // Healey & Wright

// LED segment definitions
#define LED_WHITE_START 0
#define LED_WHITE_END 58
#define LED_WRIGHT_START 58
#define LED_WRIGHT_END 78
#define LED_RETURN_OFFSET 78  // Return traffic starts at LED 78

// Variables
int intensity = 125;
unsigned long lastAPIUpdate = 0;
unsigned long lastBusMovement = 0;

// Route cycling for Button 1
const char* routes[] = {"TEAL", "GREEN", "GREENHOPPER", "ILLINI", "BLUE", "YELLOW", "YELLOWHOPPER", "RED"};
const int NUM_ROUTES = 8;
int currentRouteIndex = -1;
bool showingRouteColor = false;

// Bus tracking structure
struct Bus {
  char route_id[20];
  char direction[20];
  double lat;
  double lon;
  float led_position;
  bool active;
};

Bus buses[75];
int activeBusCount = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

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
  Serial.println("Route Segment: White St (2nd to Wright) + Wright St (White to Healey)");
  Serial.printf("LED Layout:\n");
  Serial.printf("  0-58: White St Eastbound\n");
  Serial.printf("  58-78: Wright St Southbound\n");
  Serial.printf("  78-136: White St Westbound (return)\n");
  Serial.printf("  136-156: Wright St Northbound (return)\n");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed - continuing without display"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("MTD Bus Tracker");
    display.println("White/Wright Route");
    display.display();
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  for (int i = 0; i < 75; i++) {
    buses[i].active = false;
  }

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting WiFi...");
  display.display();

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 50) {
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
    lastBusMovement = millis();
  } else {
    Serial.println("\nWiFi Failed!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi Failed!");
    display.display();
  }
}

void loop() {
  strip.setBrightness(125);

  // API polling every 60 seconds
  if (millis() - lastAPIUpdate > API_UPDATE_INTERVAL) {
    Serial.println("\n60 seconds passed, fetching data...");
    fetchBusData();
    lastAPIUpdate = millis();
  }

  // Interpolate bus movement every 30 seconds
  if (millis() - lastBusMovement > BUS_MOVEMENT_INTERVAL) {
    moveBuses();
    lastBusMovement = millis();
  }

  // Button handling
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  if (button1State == LOW) {
    currentRouteIndex++;
    if (currentRouteIndex >= NUM_ROUTES) {
      currentRouteIndex = -1;
      showingRouteColor = false;
      Serial.println("Button 1: Back to GPS tracking mode");
    } else {
      showingRouteColor = true;
      Serial.printf("Button 1: Showing route %s\n", routes[currentRouteIndex]);
    }
    delay(300);
  }

  if (button2State == LOW) {
    Serial.println("Button 2: Manual data refresh");
    fetchBusData();
    delay(300);
  }

  // Display mode
  if (showingRouteColor) {
    // Show entire strip in one color
    uint32_t routeColor = getRouteColor(routes[currentRouteIndex]);
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, routeColor);
    }
    strip.show();
  } else {
    // Show live GPS-mapped buses
    drawBuses();
  }

  updateDisplay();
  delay(1000);
}

bool isOnTrackedRoute(double lat, double lon) {
  // Check if bus is on White Street segment
  bool onWhiteStreet = (lat >= 40.1143 && lat <= 40.1148) && 
                       (lon >= -88.238 && lon <= -88.228);
  
  // Check if bus is on Wright Street segment
  bool onWrightStreet = (lat >= 40.1115 && lat <= 40.1148) &&
                        (lon >= -88.2295 && lon <= -88.2285);
  
  return onWhiteStreet || onWrightStreet;
}
}

bool isOutboundDirection(const char* direction) {
  String dir = String(direction);
  dir.toLowerCase();
  
  // Outbound: going away from campus (East on White, South on Wright, toward Urbana)
  if (dir.indexOf("south") >= 0 || dir.indexOf("east") >= 0 || dir.indexOf("urbana") >= 0) {
    return true;
  }
  // Return: going toward campus (West on White, North on Wright, toward Champaign)
  else if (dir.indexOf("north") >= 0 || dir.indexOf("west") >= 0 || dir.indexOf("champaign") >= 0) {
    return false;
  }
  
  // Default to outbound if unknown
  return true;
}

float calculateLEDPosition(double lat, double lon, const char* direction) {
  bool isOutbound = isOutboundDirection(direction);
  float basePosition = 0.0;
  
  // Determine if we're on White Street or Wright Street
  if (lat >= 40.1143) {
    // On White Street - map longitude to LED position
    float norm_lon = (lon - WHITE_STREET_START_LON) / (WHITE_STREET_END_LON - WHITE_STREET_START_LON);
    
    // Constrain to 0-1 range
    if (norm_lon < 0) norm_lon = 0;
    if (norm_lon > 1) norm_lon = 1;
    
    basePosition = norm_lon * (LED_WHITE_END - LED_WHITE_START);
  } 
  else {
    // On Wright Street - map latitude to LED position
    float norm_lat = (WRIGHT_TURN_LAT - lat) / (WRIGHT_TURN_LAT - WRIGHT_END_LAT);
    
    // Constrain to 0-1 range
    if (norm_lat < 0) norm_lat = 0;
    if (norm_lat > 1) norm_lat = 1;
    
    basePosition = LED_WRIGHT_START + (norm_lat * (LED_WRIGHT_END - LED_WRIGHT_START));
  }
  
  // If this is return direction (inbound), offset to LEDs 78-156
  if (!isOutbound) {
    basePosition += LED_RETURN_OFFSET;
  }
  
  // Final constraint to valid LED range
  if (basePosition < 0) basePosition = 0;
  if (basePosition >= NUM_LEDS) basePosition = NUM_LEDS - 1;
  
  return basePosition;
}

void fetchBusData() {
  Serial.println("\n=== Fetching Bus Data ===");
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

    // Reset all buses
    for (int i = 0; i < 75; i++) {
      buses[i].active = false;
    }

    int busIndex = 0;
    int onRouteCount = 0;

    for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
      if (busIndex >= 75) break;

      const char* route_id = vehicle["trip"]["route_id"];
      if (!route_id || strlen(route_id) == 0) continue;

      double lat = vehicle["location"]["lat"];
      double lon = vehicle["location"]["lon"];
      const char* direction = vehicle["trip"]["direction"];

      // Check if bus is on our tracked route segment
      if (isOnTrackedRoute(lat, lon)) {
        // Store bus data
        strcpy(buses[busIndex].route_id, route_id);
        
        if (direction) {
          strncpy(buses[busIndex].direction, direction, 19);
          buses[busIndex].direction[19] = '\0';
        } else {
          strcpy(buses[busIndex].direction, "Unknown");
        }
        
        buses[busIndex].lat = lat;
        buses[busIndex].lon = lon;
        buses[busIndex].led_position = calculateLEDPosition(lat, lon, buses[busIndex].direction);
        buses[busIndex].active = true;

        bool isOutbound = isOutboundDirection(buses[busIndex].direction);
        const char* dirLabel = isOutbound ? "Outbound" : "Return";
        
        Serial.printf("Bus %d: %s (%s) [%s] at LED %.1f (%.5f, %.5f)\n", 
                      busIndex, route_id, buses[busIndex].direction, dirLabel,
                      buses[busIndex].led_position, lat, lon);
        
        onRouteCount++;
        busIndex++;
      }
    }

    activeBusCount = onRouteCount;
    Serial.printf("\n=== Summary ===\n");
    Serial.printf("Total buses on route: %d\n", onRouteCount);
  }
  
  http.end();
}

void moveBuses() {
  Serial.println("\n--- Moving buses ---");
  
  int movedCount = 0;
  
  for (int i = 0; i < 75; i++) {
    if (buses[i].active) {
      String dir = String(buses[i].direction);
      dir.toLowerCase();
      
      float movement = 1.0;  // Default movement speed (1 LED per interval)
      
      // Determine movement based on position and direction
      if (buses[i].led_position < LED_RETURN_OFFSET) {
        // Outbound section (0-78): buses should move forward
        if (dir.indexOf("south") >= 0 || dir.indexOf("east") >= 0 || dir.indexOf("urbana") >= 0) {
          buses[i].led_position += movement;
        } else {
          // Wrong direction for this section - shouldn't happen, but handle it
          Serial.printf("Warning: Bus %s in outbound section with inbound direction\n", buses[i].route_id);
        }
      } 
      else {
        // Return section (78-156): buses should move forward
        if (dir.indexOf("north") >= 0 || dir.indexOf("west") >= 0 || dir.indexOf("champaign") >= 0) {
          buses[i].led_position += movement;
        } else {
          // Wrong direction for this section
          Serial.printf("Warning: Bus %s in return section with outbound direction\n", buses[i].route_id);
        }
      }
      
      // Check if bus has left the LED range
      if (buses[i].led_position < 0 || buses[i].led_position >= NUM_LEDS) {
        Serial.printf("Bus %s (%s) left LED range at %.1f\n", 
                      buses[i].route_id, buses[i].direction, buses[i].led_position);
        buses[i].active = false;
        activeBusCount--;
      } else {
        movedCount++;
      }
    }
  }
  
  Serial.printf("Moved %d buses, %d still active\n", movedCount, activeBusCount);
}

void drawBuses() {
  strip.clear();

  for (int i = 0; i < 75; i++) {
    if (buses[i].active) {
      int ledPos = (int)buses[i].led_position;
      
      if (ledPos >= 0 && ledPos < NUM_LEDS) {
        uint32_t color = getRouteColor(buses[i].route_id);
        strip.setPixelColor(ledPos, color);
        
        // Optional: Add a dimmer trailing LED for effect
        if (ledPos > 0 && ledPos < NUM_LEDS - 1) {
          strip.setPixelColor(ledPos - 1, strip.Color(
            (color >> 16 & 0xFF) / 4,
            (color >> 8 & 0xFF) / 4,
            (color & 0xFF) / 4
          ));
        }
      }
    }
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

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  if (showingRouteColor) {
    display.println("Route Display");
    display.println("---------------");
    display.printf("Route: %s\n", routes[currentRouteIndex]);
    display.println("\nBtn1: Next route");
    display.println("(or back to GPS)");
  } else {
    display.println("MTD GPS Tracker");
    display.println("White/Wright Rte");
    display.println("---------------");
    display.printf("Active buses: %d\n", activeBusCount);
    display.println("\n0-78: Outbound");
    display.println("78-156: Return");
    display.println("\nBtn1: Routes");
    display.println("Btn2: Refresh");
  }

  display.display();
}