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
#define PHOTORESISTOR_Analog 47
#define PHOTORESISTOR_out 39
#define BUTTON1_PIN 9
#define BUTTON2_PIN 10
#define BUTTON3_PIN 11
#define LED_PIN 3
#define LED_REDPIN 17
#define LED_GREENPIN 18
#define LED_BLUEPIN 8
#define NEOPIXEL_PIN 16
#define OLED_SDA_PIN 39
#define OLED_SCL_PIN 38
#define SSD1306_I2C_ADDRESS 0x3C

// LED strip setup
#define NUM_LEDS 1100
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Timing constants
#define API_UPDATE_INTERVAL 60000   // 60 seconds
#define ANIMATION_STEP_MS 25        // 25ms per LED (animation)


struct GPStoLED {
  const char* segment_name;
  float lat_min, lat_max; 
  float lon_min, lon_max;
  const char* orientation;  
  int led_out_start;        // going south [outbound (South/East/Urbana)]  [CHANGE THESE VALUES]
  int led_out_end;
  int led_in_start;         //going north [inbound (North/West/Champaign)] [CHANGE THESE VALUES]
  int led_in_end;
};



GPStoLED gps_mappings[] = {
  // WHITE STREET (East-West)
  {"White Third-Fourth", 40.114337, 40.114537, -88.235518, -88.233441, "EW", 0, 20, 20, 40},
  {"White Fourth-Fifth", 40.114337, 40.114537, -88.233641, -88.231997, "EW", 40, 60, 60, 80},
  {"White Fifth-Sixth", 40.114337, 40.114537, -88.232197, -88.230350, "EW", 80, 100, 100, 120},
  {"White Sixth-Wright", 40.114337, 40.114537, -88.230550, -88.228865, "EW", 120, 140, 140, 160},
  
  // WRIGHT STREET (North-South)
  {"Wright White-Stoughton", 40.113405, 40.114537, -88.229065, -88.228857, "NS", 160, 180, 180, 200},
  {"Wright Stoughton-Springfield", 40.112622, 40.113605, -88.229057, -88.228857, "NS", 200, 220, 220, 240},
  {"Wright Springfield-Healey", 40.111423, 40.112822, -88.229004, -88.228804, "NS", 240, 260, 260, 280},
  
  // GREEN STREET (East-West)
  {"Green Third-Fourth", 40.110150, 40.110369, -88.235498, -88.233441, "EW", 280, 300, 300, 320},
  {"Green Fourth-Fifth", 40.110169, 40.110389, -88.233641, -88.231958, "EW", 320, 340, 340, 360},
  {"Green Fifth-Sixth", 40.110189, 40.110389, -88.232158, -88.230288, "EW", 360, 380, 380, 400},
  {"Green Sixth-Wright", 40.110189, 40.110408, -88.230488, -88.228781, "EW", 400, 420, 420, 440},
  {"Green Wright-Mathews", 40.110208, 40.110579, -88.228981, -88.225500, "EW", 440, 460, 460, 480},
  {"Green Mathews-Goodwin", 40.110379, 40.110606, -88.225700, -88.223831, "EW", 480, 500, 500, 520},
  {"Green Goodwin-Lincoln", 40.110406, 40.110674, -88.224031, -88.219205, "EW", 520, 540, 540, 560},
  
  // UNIVERSITY AVENUE (East-West)
  {"University Gregory-Harvey", 40.116334, 40.116534, -88.220839, -88.222284, "EW", 560, 580, 580, 600},
  {"University Harvey-Goodwin", 40.116317, 40.116517, -88.222484, -88.223971, "EW", 600, 620, 620, 640},
  
  // GOODWIN AVENUE (North-South)
  {"Goodwin University-Clark", 40.115336, 40.116493, -88.224171, -88.223938, "NS", 640, 660, 660, 680},
  {"Goodwin Clark-Main", 40.114391, 40.115536, -88.224138, -88.223911, "NS", 680, 700, 700, 720},
  {"Goodwin Main-Springfield", 40.113425, 40.114591, -88.224111, -88.223911, "NS", 720, 740, 740, 760},
  
  // SPRINGFIELD AVENUE (East-West)
  {"Springfield Lincoln-Gregory", 40.112715, 40.112915, -88.219269, -88.220794, "EW", 760, 780, 780, 800},
  {"Springfield Gregory-Harvey", 40.112700, 40.112900, -88.220594, -88.222419, "EW", 800, 820, 820, 840},
  {"Springfield Harvey-Goodwin", 40.112687, 40.112887, -88.222219, -88.224084, "EW", 840, 860, 860, 880},
  
  // ILLINOIS STREET (East-West)
  {"Illinois Lincoln-Gregory", 40.108960, 40.109490, -88.219205, -88.221703, "EW", 880, 900, 900, 920},
  
  // GREGORY STREET (North-South)
  {"Gregory Illinois-Goodwin", 40.109190, 40.109490, -88.221503, -88.224000, "NS", 920, 940, 940, 960},
  
  // NEVADA STREET (East-West)
  {"Nevada Lincoln-Gregory", 40.105885, 40.106089, -88.219133, -88.221631, "EW", 960, 980, 980, 1000},
  {"Nevada Gregory-Goodwin", 40.105853, 40.106085, -88.221431, -88.223944, "EW", 1000, 1020, 1020, 1040}
};

const int NUM_GPS_MAPPINGS = sizeof(gps_mappings) / sizeof(gps_mappings[0]);


// MAJOR BUS STOPS - ALWAYS LIT UP 

struct BusStopLED {
  const char* stop_name;
  float lat, lon;
  int led_position;  // Major Stop LED POSITIONS [CHANGE THESE VALUES]
};

BusStopLED major_stops[] = {
  // Transit Plaza 
  {"Transit Plaza NW", 40.10847, -88.228957, 5},
  {"Transit Plaza NE", 40.108582, -88.228842, 6},
  {"Transit Plaza SW", 40.108202, -88.228923, 7},
  {"Transit Plaza SE", 40.108178, -88.228828, 8},
  
  // Illini Union 
  {"Illini Union South", 40.110364, -88.226834, 450},
  {"Illini Union North", 40.110447, -88.227402, 451},
  
  // ISR 
  {"ISR South", 40.10902, -88.22176, 550},
  {"ISR North", 40.10914, -88.22209, 551},
  
  // Krannert Center
  {"Krannert Center", 40.108291, -88.223785, 600},
  
  // White & Wright
  {"White & Wright", 40.114438, -88.228912, 140},
  
  // Stoughton & Wright
  {"Stoughton & Wright", 40.113602, -88.22901, 200},
  
  // Healey & Wright
  {"Healey & Wright E", 40.112038, -88.228853, 260},
  {"Healey & Wright NW", 40.111788, -88.228989, 261},
  
  // Green & Goodwin 
  {"Green & Goodwin SW", 40.11044, -88.224182, 500},
  {"Green & Goodwin NW", 40.110587, -88.224692, 501}
};
// might need to add more stops, these are the main ones (i think). 

const int NUM_MAJOR_STOPS = sizeof(major_stops) / sizeof(major_stops[0]);

// Variables
int intensity = 125;
unsigned long lastAPIUpdate = 0;

// Route cycling
const char* routes[] = {"TEAL", "GREEN", "GREENHOPPER", "ILLINI", "BLUE", "YELLOW", "YELLOWHOPPER", "RED"};
const int NUM_ROUTES = 8;
int currentRouteIndex = -1;
bool showingRouteColor = false;

// Bus tracking with animation support
struct Bus {
  char route_id[20];
  char direction[20];
  double lat;
  double lon;
  float current_led;      // Current position (to show moving from point to next point with animation)
  float target_led;       // Target position from API 
  bool active;
  bool animating;         
};

Bus buses[75];
int activeBusCount = 0;
bool busesAnimating = false;

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
  Serial.printf("GPS Mappings: %d\n", NUM_GPS_MAPPINGS);
  Serial.printf("Major Bus Stops: %d\n", NUM_MAJOR_STOPS);
  Serial.printf("Total LEDs: %d\n", NUM_LEDS);
  Serial.println("Features: GPS interpolation, smooth animation, bus stops always lit");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("MTD Bus Tracker");
    display.println("Smooth Animation");
    display.printf("Stops: %d\n", NUM_MAJOR_STOPS);
    display.display();
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_Analog, INPUT);

  for (int i = 0; i < 75; i++) {
    buses[i].active = false;
    buses[i].animating = false;
    buses[i].current_led = -1;
    buses[i].target_led = -1;
  }

  WiFi.begin(ssid, password);
  
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

    fetchBusData();
    lastAPIUpdate = millis();
  } else {
    Serial.println("\nWiFi Failed!");
  }
}

void loop() {
  strip.setBrightness(125);

  // Check if it's time to fetch new data
  if (millis() - lastAPIUpdate > API_UPDATE_INTERVAL) {
    Serial.println("\n60 seconds passed, fetching data...");
    fetchBusData();
    lastAPIUpdate = millis();
  }

  // Animate buses if any are animating
  if (busesAnimating) {
    animateBuses();
  }

  // Button handling
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  if (button1State == LOW) {
    currentRouteIndex++;
    if (currentRouteIndex >= NUM_ROUTES) {
      currentRouteIndex = -1;
      showingRouteColor = false;
      Serial.println("Button 1: Back to GPS");
    } else {
      showingRouteColor = true;
      Serial.printf("Button 1: Route %s\n", routes[currentRouteIndex]);
    }
    delay(300);
  }

  if (button2State == LOW) {
    Serial.println("Button 2: Manual refresh");
    fetchBusData();
    delay(300);
  }

  if (showingRouteColor) {
    uint32_t routeColor = getRouteColor(routes[currentRouteIndex]);
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, routeColor);
    }
    strip.show();
  } else {
    drawBuses();
  }

  updateDisplay();
  delay(10); 
}

// Find GPS mapping for a given lat/lon
int findGPSMapping(double lat, double lon) {
  for (int i = 0; i < NUM_GPS_MAPPINGS; i++) {
    if (lat >= gps_mappings[i].lat_min && lat <= gps_mappings[i].lat_max &&
        lon >= gps_mappings[i].lon_min && lon <= gps_mappings[i].lon_max) {
      return i;
    }
  }
  return -1;
}

// Determine if direction is outbound (South/East/Urbana) or inbound (North/West/Champaign)
bool isOutboundDirection(const char* direction) {
  if (!direction) return true;
  
  String dir = String(direction);
  dir.toLowerCase();
  
  if (dir.indexOf("south") >= 0 || dir.indexOf("east") >= 0 || dir.indexOf("urbana") >= 0) {
    return true;
  }
  else if (dir.indexOf("north") >= 0 || dir.indexOf("west") >= 0 || dir.indexOf("champaign") >= 0) {
    return false;
  }
  
  return true;
}

// Calculate LED position with GPS interpolation (Option B - smooth)
float calculateLEDPosition(double lat, double lon, int mapping_idx, const char* direction) {
  if (mapping_idx < 0 || mapping_idx >= NUM_GPS_MAPPINGS) {
    return -1;
  }
  
  GPStoLED mapping = gps_mappings[mapping_idx];
  bool isOutbound = isOutboundDirection(direction);
  
  int led_start = isOutbound ? mapping.led_out_start : mapping.led_in_start;
  int led_end = isOutbound ? mapping.led_out_end : mapping.led_in_end;
  
  float position = led_start;
  
  if (strcmp(mapping.orientation, "EW") == 0) {
    // East-West: use longitude
    float lon_range = mapping.lon_max - mapping.lon_min;
    if (abs(lon_range) < 0.000001) {
      position = led_start;
    } else {
      float norm = (lon - mapping.lon_min) / lon_range;
      
      // For westbound (inbound), reverse direction
      if (!isOutbound) {
        norm = 1.0 - norm;
      }
      
      if (norm < 0) norm = 0;
      if (norm > 1) norm = 1;
      position = led_start + (norm * (led_end - led_start));
    }
  } 
  else if (strcmp(mapping.orientation, "NS") == 0) {
    // North-South: use latitude
    float lat_range = mapping.lat_max - mapping.lat_min;
    if (abs(lat_range) < 0.000001) {
      position = led_start;
    } else {
      float norm = (mapping.lat_max - lat) / lat_range;
      
      // For northbound (inbound), reverse direction
      if (!isOutbound) {
        norm = 1.0 - norm;
      }
      
      if (norm < 0) norm = 0;
      if (norm > 1) norm = 1;
      position = led_start + (norm * (led_end - led_start));
    }
  }
  
  if (position < led_start) position = led_start;
  if (position >= led_end) position = led_end - 1;
  
  return position;
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

    // Mark all buses as inactive first
    for (int i = 0; i < 75; i++) {
      buses[i].active = false;
    }

    int busIndex = 0;
    int onRouteCount = 0;
    int totalBusesScanned = 0;
    int outboundCount = 0;
    int inboundCount = 0;

    for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
      totalBusesScanned++;
      
      if (busIndex >= 75) break;

      const char* route_id = vehicle["trip"]["route_id"];
      if (!route_id || strlen(route_id) == 0) continue;

      double lat = vehicle["location"]["lat"];
      double lon = vehicle["location"]["lon"];
      const char* direction = vehicle["trip"]["direction"];

      int mapping_idx = findGPSMapping(lat, lon);
      
      if (mapping_idx >= 0) {
        float new_led_position = calculateLEDPosition(lat, lon, mapping_idx, direction);
        
        if (new_led_position >= 0) {
          strcpy(buses[busIndex].route_id, route_id);
          
          if (direction) {
            strncpy(buses[busIndex].direction, direction, 19);
            buses[busIndex].direction[19] = '\0';
          } else {
            strcpy(buses[busIndex].direction, "Unknown");
          }
          
          buses[busIndex].lat = lat;
          buses[busIndex].lon = lon;
          buses[busIndex].active = true;
          
          // Set up animation from current to target position
          if (buses[busIndex].current_led < 0) {
            // New bus, no animation needed
            buses[busIndex].current_led = new_led_position;
            buses[busIndex].target_led = new_led_position;
            buses[busIndex].animating = false;
          } else {
            // Existing bus, animate to new position
            buses[busIndex].target_led = new_led_position;
            buses[busIndex].animating = true;
            busesAnimating = true;
          }

          bool isOutbound = isOutboundDirection(direction);
          if (isOutbound) outboundCount++;
          else inboundCount++;

          Serial.printf("Bus %d: %s (%s) [%s] on %s at LED %.1f\n", 
                        busIndex, route_id, buses[busIndex].direction,
                        isOutbound ? "OUT" : "IN",
                        gps_mappings[mapping_idx].segment_name,
                        new_led_position);
          
          onRouteCount++;
          busIndex++;
        }
      }
    }

    activeBusCount = onRouteCount;
    Serial.printf("\n=== Summary ===\n");
    Serial.printf("Total buses: %d\n", totalBusesScanned);
    Serial.printf("On tracked routes: %d\n", onRouteCount);
    Serial.printf("  Outbound: %d\n", outboundCount);
    Serial.printf("  Inbound: %d\n", inboundCount);
  }
  
  http.end();
}

// Animate all buses simultaneously (moving window with fade)
void animateBuses() {
  static unsigned long lastAnimationStep = 0;
  
  if (millis() - lastAnimationStep < ANIMATION_STEP_MS) {
    return;  // sitll not the time for next animation 
  }
  
  lastAnimationStep = millis();
  bool anyBusAnimating = false;
  
  // Move all animating buses one step
  for (int i = 0; i < 75; i++) {
    if (buses[i].active && buses[i].animating) {
      float diff = buses[i].target_led - buses[i].current_led;
      
      if (abs(diff) < 0.5) {
        // Close enough, snap to target
        buses[i].current_led = buses[i].target_led;
        buses[i].animating = false;
      } else {
        // Move one step toward target
        if (diff > 0) {
          buses[i].current_led += 1.0;
        } else {
          buses[i].current_led -= 1.0;
        }
        anyBusAnimating = true;
      }
    }
  }
  
  busesAnimating = anyBusAnimating;
}

void drawBuses() {
  strip.clear();

  // Draw major bus stops (always on, dim white)
  for (int i = 0; i < NUM_MAJOR_STOPS; i++) {
    int led = major_stops[i].led_position;
    if (led >= 0 && led < NUM_LEDS) {
      strip.setPixelColor(led, strip.Color(20, 20, 20));  // Dim white
    }
  }

  // Draw active buses with trail effect
  for (int i = 0; i < 75; i++) {
    if (buses[i].active) {
      int ledPos = (int)buses[i].current_led;
      
      if (ledPos >= 0 && ledPos < NUM_LEDS) {
        uint32_t color = getRouteColor(buses[i].route_id);
        
        // Main bus LED (bright)
        strip.setPixelColor(ledPos, color);
        
        // Trailing LED (dim)
        if (ledPos > 0) {
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
  } else {
    display.println("MTD Bus Tracker");
    display.println("Live Tracking");
    display.println("---------------");
    display.printf("Active: %d\n", activeBusCount);
    display.printf("Animating: %s\n", busesAnimating ? "YES" : "NO");
    display.println("\nBtn1: Routes");
    display.println("Btn2: Refresh");
  }

  display.display();
}
