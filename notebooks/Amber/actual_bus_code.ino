#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <vector>

// WiFi credentials
const char* ssid = "VLASSOV";
const char* password = "11111111";

// MTD API endpoint
const char* apiURL = "https://developer.mtd.org/api/v2.2/json/GetVehicles?key=13d4773418a343cc902e31242d674e67";

// Pin definitions
#define PHOTORESISTOR_Analog 4
#define PHOTORESISTOR_out 5
#define BUTTON1_PIN 15
#define BUTTON2_PIN 7
#define BUTTON3_PIN 6
#define LED_REDPIN 8
#define LED_GREENPIN 17
#define LED_BLUEPIN 18
#define NEOPIXEL_PIN 2
#define SSD1306_I2C_ADDRESS 0x3C

// LED strip setup
#define NUM_LEDS 874
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Timing constants
#define API_UPDATE_INTERVAL 60000   // 60 seconds
#define ANIMATION_STEP_MS 25        // 25ms per LED (animation)

struct LEDRange {
    int start;
    int end;   // inclusive
};

struct GPStoLED {
    const char* segment_name;
    std::vector<LEDRange> ranges;
    double minLon, maxLon;
    double minLat, maxLat;
    bool reverse;
};

std::vector<GPStoLED> gps_mappings = {      //east-to-west or south-to-north is false

    { "White Third-Wright",     //good
      { {0,49} },
      -88.235908, -88.229173,     
      40.114380, 40.114552, false},

    { "Wright White-Armory",     //good 
      { {50,137} },
      -88.229142, -88.228681,
      40.105711, 40.114450, true },

    { "Armory Wright-Fourth",   
      { {138,173} },
      -88.233448, -88.228836,   //good
      40.105302, 40.105646, true},

    { "Chalmers Wright-Fourth",   //good  
      { {174,207} },    
      -88.233297, -88.228967,
      40.106706, 40.107051, true},

    { "Sixth Chalmers-End",         //good
      { {208,219}, {150,150}, {220,231}, {351,351}, {232,241} },
      -88.230490, -88.230079,
      40.103051, 40.106768, true},

    { "Daniel End-Fourth",
      { {242,261} },            //good
      -88.236070, -88.233489,
      40.107828, 40.108126, false},

    { "Fourth Daniel-End",      //good
      { {261,261}, {262,285}, {173,173}, {286,297}, {327,327}, {298,307} },
      -88.233680, -88.233263,
      40.102764, 40.107876, true},

    { "Gregory End-Dorner",   //good
      { {308,413} },
      -88.236167, -88.222142,
      40.104058, 40.104366, false},

    { "Dorner Gregory-End",   //good
      { {414,425} },
      -88.221948, -88.221748,
      40.103153, 40.104181, true},

    { "Goodwin Gregory-University",   //good
      { {426,545} },
      -88.224016, -88.223791,
      40.104266, 40.116273, false},

    { "University Goodwin-Lincoln",   //good
      { {546,581} },
      -88.219350, -88.223979,
      40.116325, 40.116549, false},

    { "Springfield Lincoln-Goodwin",
      { {582,619} },
      -88.224168, -88.219200,
      40.112657, 40.112927, false},

    { "Illinois Goodwin-Lincoln",   //good
      { {620,653} },
      -88.223725, -88.218688,
      40.108985, 40.109159, false},

    { "Lincoln Illinois-End",     //good
      { {654,713} },
      -88.219449, -88.219086,
      40.103132, 40.109000, true},

    { "Nevada Lincoln-Goodwin",
      { {714,747} },
      -88.223720, -88.219316,
      40.105923, 40.106074, true},

    { "Green Fourth-Lincoln",
      { {748,799}, {91,91}, {800,835}, {488,488}, {836,873} },
      -88.236069, -88.218680,
      40.110148, 40.110771, false}
};

const int NUM_GPS_MAPPINGS = gps_mappings.size();

// Variables
int intensity = 125;
unsigned long lastAPIUpdate = 0;

// Route cycling
const char* routes[] = {"TEAL", "GREEN", "ILLINI", "YELLOW" ,"RED", "BLUE", "BROWN", "GOLD", "SILVER", "BRONZE", "RAVEN", "LINK"};
const int NUM_ROUTES = 12;
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
  Serial.printf("Total LEDs: %d\n", NUM_LEDS);
  Serial.println("Features: GPS interpolation, smooth animation, bus stops always lit");

  //pinMode(LED_PIN, OUTPUT);
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

  Serial.print("ESP32-S3 MAC Address: ");
  Serial.println(WiFi.macAddress());
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
    //animateBuses();
  }

  // Button handling
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  if (button1State == HIGH) {
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

  if (button2State == HIGH) {
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

  //updateDisplay();
  delay(10); 
}


int ledFromNormalized(const GPStoLED& road, float t)
{
    int totalLEDs = 0;
    for (auto& r : road.ranges)
        totalLEDs += (r.end - r.start + 1);

    if (totalLEDs <= 0)
        return -1;

    // If reversed, invert t
    if (road.reverse) {
        t = 1.0f - t;
    }

    int idx = floor(t * (totalLEDs - 1));

    for (auto& r : road.ranges)
    {
        int len = r.end - r.start + 1;
        if (idx < len)
            return r.start + idx;

        idx -= len;
    }

    return -1;
}

// ===============================
// Main GPS → LED mapping function
// ===============================
int calculateLEDPosition(double lat, double lon)
{
    for (auto& road : gps_mappings)
    {
        bool inLat = (lat >= road.minLat && lat <= road.maxLat);
        bool inLon = (lon >= road.minLon && lon <= road.maxLon);
        if (!inLat || !inLon)
            continue;

        float tLat = (road.maxLat != road.minLat)
                     ? (lat - road.minLat) / (road.maxLat - road.minLat)
                     : 0;

        float tLon = (road.maxLon != road.minLon)
                     ? (lon - road.minLon) / (road.maxLon - road.minLon)
                     : 0;

        bool horizontal = fabs(road.maxLon - road.minLon) >
                          fabs(road.maxLat - road.minLat);

        float t = horizontal ? tLon : tLat;

        if (t < 0) t = 0;
        if (t > 1) t = 1;

        return ledFromNormalized(road, t);
    }

    return -1;
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

  // Draw active buses with trail effect
  for (int i = 0; i < 25; i++) {
    if (buses[i].active) {
      int ledPos = (int)buses[i].current_led;
      
      if (ledPos >= 0 && ledPos < NUM_LEDS) {
        uint32_t color = getRouteColor(buses[i].route_id);
        // Main bus LED (bright)
        strip.setPixelColor(ledPos, color);
      }
    }
  }

  strip.show();
}

uint32_t getRouteColor(const char* route_id) {
  if (strcmp(route_id, "TEAL") == 0) return strip.Color(0, 225, 110);
  if (strcmp(route_id, "GREEN") == 0 || strcmp(route_id, "GREENHOPPER") == 0 || strcmp(route_id, "GREEN EXPRESS") == 0) return strip.Color(0, 255, 0);
  if (strcmp(route_id, "ILLINI") == 0) return strip.Color(255, 0, 255);
  if (strcmp(route_id, "YELLOW") == 0 || strcmp(route_id, "YELLOWHOPPER") == 0) return strip.Color(255, 150, 0);
  if (strcmp(route_id, "RED") == 0) return strip.Color(255, 0, 0);
  if (strcmp(route_id, "BLUE") == 0) return strip.Color(0, 0, 255);
  if (strcmp(route_id, "BROWN") == 0) return strip.Color(255, 35, 0);
  if (strcmp(route_id, "GOLD") == 0 || strcmp(route_id, "GOLDHOPPER") == 0) return strip.Color(255, 85, 0);
  if (strcmp(route_id, "SILVER") == 0) return strip.Color(200, 200, 200);
  if (strcmp(route_id, "BRONZE") == 0) return strip.Color(255, 55, 5);
  if (strcmp(route_id, "RAVEN") == 0) return strip.Color(255, 50, 50);
  if (strcmp(route_id, "LINK") == 0) return strip.Color(0, 255, 255);
  return strip.Color(255, 255, 255);
} 

void fetchBusData() {
  int totalBusesScanned = 0;
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
  int onRouteCount = 0;
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
      buses[i].animating = false;
      buses[i].current_led = -1;
      buses[i].target_led = -1;
    }

    int busIndex = 0;
    

    for (JsonObject vehicle : doc["vehicles"].as<JsonArray>()) {
      totalBusesScanned++;
      
      if (busIndex >= 75) break;

      const char* fullRouteID = vehicle["trip"]["route_id"];

      if (!fullRouteID || strlen(fullRouteID) == 0) continue;

      char routeID[20];  
      int i = 0;
      while (fullRouteID[i] != ' ' && fullRouteID[i] != '\0' && i < sizeof(routeID)-1) {
          routeID[i] = fullRouteID[i];
          i++;
      }
      routeID[i] = '\0';  // null terminate


      double lat = vehicle["location"]["lat"];
      double lon = vehicle["location"]["lon"];
      const char* direction = vehicle["trip"]["direction"];

      int new_led_position = calculateLEDPosition(lat, lon);        

      if (new_led_position >= 0) {
        Serial.print("Route: ");
        Serial.print(routeID);
        Serial.print(" ");

        Serial.print("Latitude: ");
        Serial.print(lat, 6);
        Serial.print(" ");

        Serial.print("Longitude: ");
        Serial.print(lon, 6);
        Serial.print(" ");

        Serial.print("LED: ");
        Serial.print(new_led_position);
        Serial.println(" ");

        strcpy(buses[busIndex].route_id, routeID);

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

          onRouteCount++;
          busIndex++;
        }
      }
    }

    activeBusCount = onRouteCount;
    Serial.printf("\n=== Summary ===\n");
    Serial.printf("Total buses: %d\n", totalBusesScanned);
    Serial.printf("On tracked routes: %d\n", onRouteCount);

  
  http.end();
}

