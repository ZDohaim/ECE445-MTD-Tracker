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
#define MENU_STRIP_PIN 1
#define BUZZER_PIN 16
#define SSD1306_I2C_ADDRESS 0x3C
#define freq 3000

// LED strip setup
#define NUM_LEDS 874
#define MENU_LEDS 12
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel menuStrip(MENU_LEDS, MENU_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Timing constants
#define API_UPDATE_INTERVAL 10000  // 60 seconds
#define ANIMATION_STEP_MS 25       // 25ms per LED (animation)
#define MENU_TIMEOUT 10000         // 10 seconds timeout
#define FLASH_INTERVAL 250         // Flash interval for menu mode (ms)
#define HOLD_THRESHOLD 2000        // 2 second hold
#define ROUTE_DISPLAY_TIME 5000   // 15 seconds to display route

// Button variables for menu system
// bool lastButtonState = HIGH;
// bool buttonState = HIGH;
// unsigned long lastDebounceTime = 0;
 unsigned long debounceDelay = 50;
// unsigned long buttonPressStart = 0;
// unsigned long lastMenuActivity = 0;
// unsigned long lastFlashTime = 0;
// bool buttonHeld = false;
// bool flashState = false;
 unsigned long routeDisplayEnd;

const unsigned long shortPressTime = 0;   // 0.3 seconds
const unsigned long longPressTime  = 500;  // 1.0 seconds
unsigned long lastDebounceTime = 0;
unsigned long pressStartTime   = 0;
bool lastButtonStateRoute = LOW;
bool buttonState     = LOW;
bool shortPress = false;
bool longPress  = false;

unsigned long lastFlashTime = 0;
bool ledOn = false;  // current flash state


#define BRIGHTNESS_LOW 10
#define BRIGHTNESS_MEDLOW 75
#define BRIGHTNESS_MED 150
#define BRIGHTNESS_HIGH 255
#define PHOTORESISTOR_Analog 4
#define SMOOTHING_SAMPLES 10


int brightnessButtonCount = 0;
int photoReadings[SMOOTHING_SAMPLES];
int photoIndex = 0;
int photoTotal = 0;


enum BrightnessMode {
  BRIGHTNESS_LOW_MODE,
  BRIGHTNESS_MEDLOW_MODE,
  BRIGHTNESS_MED_MODE,
  BRIGHTNESS_HIGH_MODE,
  BRIGHTNESS_AUTO_MODE
};


// =============================================================
enum ThemesColor {
  THEME_DEFAULT,
  CHRISTMAS,
  ILLINI,
  SAINTPT,
  HALLOWEEN,
  AMERICA
};

ThemesColor currentTheme = AMERICA;

struct RGB {
    uint8_t r, g, b;
};

struct Theme {
    RGB routes[12];    // 12 route colors per theme
};

RGB RGBrouteColors[12];

Theme defaultTheme = {
    {
        { 0,   225, 110 },  // TEAL
        { 0,   255, 0   },  // GREEN
        { 255, 0,   255 },  // ILLINI
        { 255, 150, 0   },  // YELLOW
        { 255, 0,   0   },  // RED
        { 0,   0,   255 },  // BLUE
        { 255, 35,  0   },  // BROWN
        { 255, 85,  0   },  // GOLD
        { 200, 200, 200 },  // SILVER
        { 255, 55,  5   },  // BRONZE
        { 255, 50,  50  },  // RAVEN
        { 0,   255, 255 }   // LINK
    }
};


Theme christmasTheme = {
    {
        {0,255,0}, {255,0,0}, {0,255,0}, {255,0,0},
        {255,0,0}, {0,255,0}, {255,0,0}, {0,255,0},
        {255,0,0}, {0,255,0}, {255,0,0}, {0,255,0}
    }
};

Theme illiniTheme = {
    {
        {255,35,0}, {0,40,255}, {255,35,0}, {0,40,255},
        {255,35,0}, {0,40,255}, {255,35,0}, {0,40,255},
        {255,35,0}, {0,40,255}, {255,35,0}, {0,40,255}
    }
};

Theme saintPTTheme = {
    {
        {255,255,255}, {0,150,0}, {0,255,0}, {0,200,0},
        {255,255,255}, {0,150,0}, {0,255,0}, {0,200,0},
        {255,255,255}, {0,150,0}, {0,255,0}, {0,200,0}
    }
};

Theme halloweenTheme = {
    {
        {255,35,0}, {255,0,255}, {255,35,0}, {255,0,255},
        {255,35,0}, {255,0,255}, {255,35,0}, {255,0,255},
        {255,35,0}, {255,0,255}, {255,35,0}, {255,0,255}
    }
};

Theme americaTheme = {
    {
        {255,0,0}, {0,0,255}, {255,0,0}, {255,255,255},
        {255,0,0}, {0,0,255}, {255,255,255}, {255,0,0},
        {0,0,255}, {255,255,255}, {255,0,0}, {0,0,255}
    }
};


Theme allThemes[] = {
    defaultTheme,     // DEFAULT
    christmasTheme,   // CHRISTMAS
    illiniTheme,      // ILLINI
    saintPTTheme,     // SAINTPT
    halloweenTheme,   // HALLOWEEN
    americaTheme      // AMERICA
};




// Mode variables
enum Mode {
  BUS_DATA_MODE,
  MENU_MODE,
  ROUTE_DISPLAY_MODE
};

Mode currentMode = BUS_DATA_MODE;
BrightnessMode currentBrightnessMode = BRIGHTNESS_MED_MODE;

int menuSelection = 0;

struct LEDRange {
  int start;
  int end;  // inclusive
};

struct GPStoLED {
  const char* segment_name;
  std::vector<LEDRange> ranges;
  double minLon, maxLon;
  double minLat, maxLat;
  bool reverse;
};

std::vector<GPStoLED> gps_mappings = {
  { "White Third-Wright",
    { { 0, 49 } },
    -88.235908,
    -88.229173,
    40.114380,
    40.114552,
    false },

  { "Wright White-Armory",
    { { 50, 137 } },
    -88.229142,
    -88.228681,
    40.105711,
    40.114450,
    true },

  { "Armory Wright-Fourth",
    { { 138, 173 } },
    -88.233448,
    -88.228836,
    40.105302,
    40.105646,
    true },

  { "Chalmers Wright-Fourth",
    { { 174, 207 } },
    -88.233297,
    -88.228967,
    40.106706,
    40.107051,
    true },

  { "Sixth Chalmers-End",
    { { 208, 219 }, { 150, 150 }, { 220, 231 }, { 351, 351 }, { 232, 241 } },
    -88.230490,
    -88.230079,
    40.103051,
    40.106768,
    true },

  { "Daniel End-Fourth",
    { { 242, 261 } },
    -88.236070,
    -88.233489,
    40.107828,
    40.108126,
    false },

  { "Fourth Daniel-End",
    { { 261, 261 }, { 262, 285 }, { 173, 173 }, { 286, 297 }, { 327, 327 }, { 298, 307 } },
    -88.233680,
    -88.233263,
    40.102764,
    40.107876,
    true },

  { "Gregory End-Dorner",
    { { 308, 413 } },
    -88.236167,
    -88.222142,
    40.104058,
    40.104366,
    false },

  { "Dorner Gregory-End",
    { { 414, 425 } },
    -88.221948,
    -88.221748,
    40.103153,
    40.104181,
    true },

  { "Goodwin Gregory-University",
    { { 426, 545 } },
    -88.224016,
    -88.223791,
    40.104266,
    40.116273,
    false },

  { "University Goodwin-Lincoln",
    { { 546, 581 } },
    -88.219350,
    -88.223979,
    40.116325,
    40.116549,
    false },

  { "Springfield Lincoln-Goodwin",
    { { 582, 619 } },
    -88.224168,
    -88.219200,
    40.112657,
    40.112927,
    false },

  { "Illinois Goodwin-Lincoln",
    { { 620, 653 } },
    -88.223725,
    -88.218688,
    40.108985,
    40.109159,
    false },

  { "Lincoln Illinois-End",
    { { 654, 713 } },
    -88.219449,
    -88.219086,
    40.103132,
    40.109000,
    true },

  { "Nevada Lincoln-Goodwin",
    { { 714, 747 } },
    -88.223720,
    -88.219316,
    40.105923,
    40.106074,
    true },

  { "Green Fourth-Lincoln",
    { { 748, 799 }, { 91, 91 }, { 800, 835 }, { 488, 488 }, { 836, 873 } },
    -88.236069,
    -88.218680,
    40.110148,
    40.110771,
    false }
};

const int NUM_GPS_MAPPINGS = gps_mappings.size();

// Variables
int intensity = 125;
unsigned long lastAPIUpdate = 0;

// Route names
const char* routeNames[] = {
  "TEAL", "GREEN", "ILLINI", "YELLOW", "RED", "BLUE",
  "BROWN", "GOLD", "SILVER", "BRONZE", "RAVEN", "LINK"
};
const int NUM_ROUTES = 12;

// Route colors for menu strip
uint32_t routeColors[] = {
  menuStrip.Color(0, 225, 110),    // TEAL
  menuStrip.Color(0, 255, 0),      // GREEN
  menuStrip.Color(255, 0, 255),    // ILLINI (Magenta)
  menuStrip.Color(255, 150, 0),    // YELLOW
  menuStrip.Color(255, 0, 0),      // RED
  menuStrip.Color(0, 0, 255),      // BLUE
  menuStrip.Color(255, 35, 0),     // BROWN
  menuStrip.Color(255, 85, 0),     // GOLD
  menuStrip.Color(200, 200, 200),  // SILVER
  menuStrip.Color(255, 55, 5),     // BRONZE
  menuStrip.Color(255, 50, 50),    // RAVEN
  menuStrip.Color(0, 255, 255)     // LINK (Cyan)
};

// Bus tracking
struct Bus {
  char route_id[20];
  char direction[20];
  double lat;
  double lon;
  float current_led;
  float target_led;
  bool active;
  bool animating;
};

Bus buses[75];
int activeBusCount = 0;
bool busesAnimating = false;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Initialize buttons FIRST before anything else
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(PHOTORESISTOR_Analog, INPUT);

  ledcSetup(0, 9000, 8);    // Channel 0, 1000 Hz frequency, 8-bit resolution
  ledcAttachPin(BUZZER_PIN, 0); // Attach pin 16 to channel 0

  // Give pull-ups time to stabilize
  delay(100);

  strip.begin();
  initBrightness();
  strip.clear();
  strip.show();

  menuStrip.begin();
  menuStrip.setBrightness(25);
  menuStrip.clear();
  showAllRoutes();
  menuStrip.show();


  pinMode(LED_GREENPIN, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_GREENPIN, HIGH);
    delay(100);
    digitalWrite(LED_GREENPIN, LOW);
    delay(100);
  }
  digitalWrite(LED_GREENPIN, HIGH);


  Serial.println("\n=== MTD Bus Tracker Initializing ===");
  Serial.printf("GPS Mappings: %d\n", NUM_GPS_MAPPINGS);
  Serial.printf("Total LEDs: %d\n", NUM_LEDS);

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
  Serial.println("\n=== System Ready ===");
  ledcWriteTone(0, freq);  // 1000 Hz tone
  Serial.println("buzzing");
  delay(100);
  ledcWriteTone(0, 0); 
  ledcWriteTone(0, freq);  // 1000 Hz tone
  Serial.println("buzzing");
  delay(100);
  ledcWriteTone(0, 0); 



}



void exitRouteDisplay(){
  currentMode = BUS_DATA_MODE;
  strip.clear();
  strip.show();
}




void loop() {
  // strip.setBrightness(125);

  // Handle button input
  handleRouteButton();

  if(currentMode == BUS_DATA_MODE){
    if(shortPress){
      shortPress = false;
      Serial.println("Going to the MENU Mode: ");
      currentMode = MENU_MODE;
      
    }
  }


  // Mode-specific behavior
  switch (currentMode) {
    case BUS_DATA_MODE:
      // Check if it's time to fetch new data
      if (millis() - lastAPIUpdate > API_UPDATE_INTERVAL) {
        Serial.println("\n60 seconds passed, fetching data...");
        fetchBusData();
        lastAPIUpdate = millis();
        
      }

      if (digitalRead(BUTTON2_PIN) == HIGH) {  
        ledcWriteTone(0, freq);  // 1000 Hz tone
        Serial.println("buzzing");
        delay(100);
        ledcWriteTone(0, 0); 
        Serial.println("brightness button pressed");
        cycleBrightness();
        delay(100);
      }

      if (digitalRead(BUTTON3_PIN) == HIGH) {  
        ledcWriteTone(0, freq);  // 1000 Hz tone
        Serial.println("buzzing");
        delay(100);
        ledcWriteTone(0, 0); 
        Serial.println("theme button pressed");
        cycleThemes();
        delay(100);
      }

      drawBuses();
      break;

    case MENU_MODE:
      //Serial.println("\n>>> ENTERING MENU MODE <<<");
      enterMenuMode();
     

      break;

    case ROUTE_DISPLAY_MODE:
      // Check if display time is over
      if (millis() - routeDisplayEnd > ROUTE_DISPLAY_TIME) {
        Serial.println("\nRoute display time over, returning to bus data mode");
        exitRouteDisplay();
        
        menuStrip.clear();
        showAllRoutes();
        menuStrip.show();


      }
      break;
  }
  updateBrightness();
  delay(10);
}

// ===== Function to Handle route Button =====
void handleRouteButton()
{
    bool reading = digitalRead(BUTTON1_PIN);

    // Debounce
    if (reading != lastButtonStateRoute) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == HIGH) {
                // Button just pressed
                ledcWriteTone(0, freq);  // 1000 Hz tone
                Serial.println("buzzing");
                delay(100);
                ledcWriteTone(0, 0);
                pressStartTime = millis();
            } 
            else {
                // Button just released
                unsigned long pressDuration = millis() - pressStartTime;

                if (pressDuration >= longPressTime) {
                    longPress = true;
                    Serial.println("Long Press");
                } 
                else if (pressDuration >= shortPressTime) {
                    shortPress = true;
                    Serial.println("Short Press");
                }
            }
        }
    }

    lastButtonStateRoute = reading;
}



void enterMenuMode() {

  handleRouteButton();

  if(shortPress){
    shortPress = false;

    menuSelection++;
    Serial.print("Menu Selection: ");
    Serial.println(menuSelection);

    if(menuSelection > 11) menuSelection = 0;
  }

  if(longPress){
    longPress = false;

    Serial.print("Displaying route: ");
    Serial.println(routeNames[menuSelection]);
    // Enter route display mode
    currentMode = ROUTE_DISPLAY_MODE;
  
    // Display the selected route on main strip
    showAllRoutes();
    displayRoute(menuSelection);
  }
  
   
  if (millis() - lastFlashTime >= 200) {
    lastFlashTime = millis();
    ledOn = !ledOn;

    menuStrip.clear();
    if (ledOn) {
      menuStrip.setPixelColor(menuSelection,routeColors[menuSelection]);
    }
    menuStrip.show();
  }
}



void showAllRoutes() {
  // Display all route colors on menu strip as indicator
  menuStrip.clear();
  for (int i = 0; i < NUM_ROUTES; i++) {
    menuStrip.setPixelColor(i, routeColors[i]);
  }
  menuStrip.show();
}

void displayRoute(int routeIndex) {
  Serial.println("\n========================");
  Serial.print("Route: ");
  Serial.println(routeNames[routeIndex]);
  Serial.println("========================");
  uint32_t color = routeColors[routeIndex];
  strip.clear();

for(int i = 0; i < 975; i++){
    strip.setPixelColor(i,5,5,5);
  }

  strip.show();



  switch (routeIndex) {
    case 0: displayTeal(color); break;
    case 1: displayGreen(color); break;
    case 2: displayIllini(color); break;
    case 3: displayYellow(color); break;
    case 4: displayRed(color); break;
    case 5: displayBlue(color); break;
    case 6: displayBrown(color); break;
    case 7: displayGold(color); break;
    case 8: displaySilver(color); break;
    case 9: displayBronze(color); break;
    case 10: displayRaven(color); break;
    case 11: displayLink(color); break;
  }

  routeDisplayEnd = millis();
  Serial.print(routeDisplayEnd);
  Serial.println("Route display complete!");
}

void lightSegment(int start, int end, uint32_t color) {
  if (start <= end) {
    for (int i = start; i <= end; i++) {
      strip.setPixelColor(i, color);
      strip.show();
    }
  } else {
    // Reverse direction
    for (int i = start; i >= end; i--) {
      strip.setPixelColor(i, color);
      strip.show();
    }
  }
}

void lightMultipleSegments(int segments[][2], int numSegments, uint32_t color, const char* segmentName, bool flip = false) {
  Serial.print(" - ");
  Serial.println(segmentName);
  for (int i = 0; i < numSegments; i++) {
    if (flip) {
      lightSegment(segments[i][1], segments[i][0], color);
    } else {
      lightSegment(segments[i][0], segments[i][1], color);
    }
  }
}

// TEAL ROUTE
void displayTeal(uint32_t color) {
  int dorner[][2] = { { 414, 425 } };
  lightMultipleSegments(dorner, 1, color, "Dorner Gregory-End", true);
  int gregory1[][2] = { { 400, 415 } };
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", true);
  int goodwin1[][2] = { { 426, 442 }, { 443, 473 }, { 474, 488 } };
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Gregory-Green");
  int green1[][2] = { { 800, 835 } };
  lightMultipleSegments(green1, 1, color, "Green Goodwin-Wright", true);
  int wright1[][2] = { { 50, 91 } };
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  int white1[][2] = { { 0, 49 } };
  lightMultipleSegments(white1, 1, color, "White", true);
}

// GREEN ROUTE
void displayGreen(uint32_t color) {
  int green1[][2] = { { 748, 799 }, { 91, 91 }, { 800, 835 }, { 488, 488 }, { 836, 873 } };
  lightMultipleSegments(green1, 5, color, "Green Fourth-Goodwin");
  int goodwin1[][2] = { { 474, 487 }, { 443, 473 }, { 426, 442 } };
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  int gregory1[][2] = { { 352, 400 }, { 327, 351 } };
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Fourth", true);
  int fourth1[][2] = { { 298, 307 } };
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", false);
}

// ILLINI ROUTE
void displayIllini(uint32_t color) {
  int lincoln1[][2] = { { 684, 713 }, { 654, 683 } };
  lightMultipleSegments(lincoln1, 2, color, "Lincoln End-Illinois", true);
  int illinois1[][2] = { { 620, 653 } };
  lightMultipleSegments(illinois1, 1, color, "Illinois End-Goodwin", true);
  int goodwin1[][2] = { { 443, 473 }, { 426, 442 } };
  lightMultipleSegments(goodwin1, 2, color, "Goodwin Illinois-Gregory", true);
  int gregory1[][2] = { { 352, 400 }, { 327, 351 } };
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Fourth", true);
  int fourth1[][2] = { { 298, 307 } };
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", false);
  int daniel1[][2] = { { 242, 261 } };
  lightMultipleSegments(daniel1, 1, color, "Daniel End-Fourth");
  int fourth2[][2] = { { 261, 272 }, { 273, 285 } };
  lightMultipleSegments(fourth2, 2, color, "Fourth Daniel-Armory");
  int armory1[][2] = { { 173, 150 }, { 149, 138 } };
  lightMultipleSegments(armory1, 2, color, "Armory", false);
  int wright1[][2] = { { 126, 137 }, { 91, 125 } };
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  int green1[][2] = { { 91, 91 }, { 800, 835 }, { 488, 488 } };
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  int goodwin2[][2] = { { 488, 510 }, { 511, 545 } };
  lightMultipleSegments(goodwin2, 2, color, "Goodwin Green-University");
  int university1[][2] = { { 546, 581 } };
  lightMultipleSegments(university1, 1, color, "University Goodwin-Lincoln");
}

// YELLOW ROUTE
void displayYellow(uint32_t color) {
  int gregory1[][2] = { { 308, 327 } };
  lightMultipleSegments(gregory1, 1, color, "Gregory End-Fourth");
  int fourth1[][2] = { { 286, 297 }, { 327, 327 }, { 173, 173 } };
  lightMultipleSegments(fourth1, 3, color, "Fourth Gregory-Armory", true);
  int armory1[][2] = { { 150, 173 }, { 138, 149 }, { 208, 219 }, { 174, 184 } };
  lightMultipleSegments(armory1, 4, color, "Armory", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 }, { 50, 90 } };
  lightMultipleSegments(wright1, 3, color, "Wright Armory-White", true);
  int white1[][2] = { { 0, 49 } };
  lightMultipleSegments(white1, 1, color, "White", true);
}

// RED ROUTE
void displayRed(uint32_t color) {
  int lincoln1[][2] = { { 684, 713 } };
  lightMultipleSegments(lincoln1, 1, color, "Lincoln End-Nevada", true);
  int nevada1[][2] = { { 714, 747 } };
  lightMultipleSegments(nevada1, 1, color, "Nevada Lincoln-Goodwin");
  int goodwin1[][2] = { { 443, 473 }, { 474, 487 } };
  lightMultipleSegments(goodwin1, 2, color, "Goodwin Nevada-Green");
  int green1[][2] = { { 488, 488 }, { 800, 835 } };
  lightMultipleSegments(green1, 2, color, "Green Goodwin-Wright", true);
  int wright1[][2] = { { 50, 91 } };
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  int white1[][2] = { { 0, 49 } };
  lightMultipleSegments(white1, 1, color, "White", true);
}

// BLUE ROUTE
void displayBlue(uint32_t color) {
  int sixth1[][2] = { { 232, 241 }, { 351, 351 }, { 220, 231 }, { 149, 149 }, { 208, 219 }, { 174, 184 } };
  lightMultipleSegments(sixth1, 6, color, "Sixth End-Armory", true);
  int armory1[][2] = { { 138, 149 } };
  lightMultipleSegments(armory1, 1, color, "Armory", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 }, { 50, 90 }, { 0, 49 } };
  lightMultipleSegments(wright1, 4, color, "Wright Armory-White", true);
}

// BROWN ROUTE
void displayBrown(uint32_t color) {
  int sixth1[][2] = { { 232, 241 }, { 351, 351 }, { 220, 231 }, { 149, 149 }, { 208, 219 }, { 174, 184 } };
  lightMultipleSegments(sixth1, 6, color, "Sixth End-Armory", true);
  int armory1[][2] = { { 138, 149 } };
  lightMultipleSegments(armory1, 1, color, "Armory", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 }, { 50, 90 }, { 0, 49 } };
  lightMultipleSegments(wright1, 4, color, "Wright Armory-White", true);
}

// GOLD ROUTE
void displayGold(uint32_t color) {
  int gregory1[][2] = { { 308, 327 }, { 328, 351 }, { 352, 400 } };
  lightMultipleSegments(gregory1, 3, color, "Gregory End-Goodwin");
  int goodwin1[][2] = { { 426, 442 }, { 443, 473 }, { 474, 487 }, { 488, 510 } };
  lightMultipleSegments(goodwin1, 4, color, "Goodwin Gregory-Springfield");
  int springfield1[][2] = { { 582, 619 } };
  lightMultipleSegments(springfield1, 1, color, "Springfield Goodwin-End", true);
}

// SILVER ROUTE
void displaySilver(uint32_t color) {
  int dorner1[][2] = { { 414, 425 } };
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End", true);
  int gregory1[][2] = { { 401, 413 }, { 351, 400 } };
  lightMultipleSegments(gregory1, 2, color, "Gregory Dorner-Sixth", true);
  int sixth1[][2] = { { 220, 231 }, { 208, 219 }, { 174, 184 } };
  lightMultipleSegments(sixth1, 3, color, "Sixth Gregory-Armory", true);
  int armory1[][2] = { { 138, 149 } };
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 } };
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  int green1[][2] = { { 91, 91 }, { 800, 835 }, { 488, 488 } };
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  int goodwin1[][2] = { { 488, 510 } };
  lightMultipleSegments(goodwin1, 1, color, "Goodwin Green-Springfield");
  int springfield1[][2] = { { 582, 619 } };
  lightMultipleSegments(springfield1, 1, color, "Springfield Goodwin-End", true);
}

// BRONZE ROUTE
void displayBronze(uint32_t color) {
  int fourth1[][2] = { { 298, 307 }, { 327, 327 }, { 286, 297 } };
  lightMultipleSegments(fourth1, 3, color, "Fourth End-Armory", true);
  int armory1[][2] = { { 150, 173 }, { 138, 149 } };
  lightMultipleSegments(armory1, 2, color, "Armory Fourth-Wright", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 } };
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  int green1[][2] = { { 800, 835 } };
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  int goodwin1[][2] = { { 474, 488 }, { 443, 473 }, { 426, 442 } };
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  int gregory1[][2] = { { 400, 413 } };
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", false);
  int dorner1[][2] = { { 414, 425 } };
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}

// RAVEN ROUTE
void displayRaven(uint32_t color) {
  int sixth1[][2] = { { 232, 241 }, { 351, 351 }, { 220, 231 } };
  lightMultipleSegments(sixth1, 3, color, "Sixth End-Armory", true);
  int armory1[][2] = { { 138, 149 } };
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 } };
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  int green1[][2] = { { 800, 835 } };
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  int goodwin1[][2] = { { 474, 488 }, { 443, 473 }, { 426, 442 } };
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  int gregory1[][2] = { { 400, 413 } };
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", false);
  int dorner1[][2] = { { 414, 425 } };
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}

// LINK ROUTE
void displayLink(uint32_t color) {
  int sixth1[][2] = { { 232, 241 }, { 351, 351 }, { 220, 231 }, { 149, 149 }, { 208, 219 }, { 174, 184 } };
  lightMultipleSegments(sixth1, 6, color, "Sixth End-Armory", true);
  int armory1[][2] = { { 138, 149 } };
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  int wright1[][2] = { { 126, 137 }, { 91, 125 } };
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  int green1[][2] = { { 91, 91 }, { 800, 835 }, { 488, 488 } };
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  int goodwin2[][2] = { { 488, 510 }, { 511, 545 } };
  lightMultipleSegments(goodwin2, 2, color, "Goodwin Green-University");
  int university1[][2] = { { 546, 581 } };
  lightMultipleSegments(university1, 1, color, "University Goodwin-Lincoln");
}

// ===== GPS and Bus Functions (unchanged) =====

int ledFromNormalized(const GPStoLED& road, float t) {
  int totalLEDs = 0;
  for (auto& r : road.ranges)
    totalLEDs += (r.end - r.start + 1);

  if (totalLEDs <= 0)
    return -1;

  if (road.reverse) {
    t = 1.0f - t;
  }

  int idx = floor(t * (totalLEDs - 1));

  for (auto& r : road.ranges) {
    int len = r.end - r.start + 1;
    if (idx < len)
      return r.start + idx;
    idx -= len;
  }

  return -1;
}

int calculateLEDPosition(double lat, double lon) {
  for (auto& road : gps_mappings) {
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

    bool horizontal = fabs(road.maxLon - road.minLon) > fabs(road.maxLat - road.minLat);

    float t = horizontal ? tLon : tLat;

    if (t < 0) t = 0;
    if (t > 1) t = 1;

    return ledFromNormalized(road, t);
  }

  return -1;
}

void animateBuses() {
  static unsigned long lastAnimationStep = 0;
  if (millis() - lastAnimationStep < ANIMATION_STEP_MS) {
    return;
  }
  lastAnimationStep = millis();
  bool anyBusAnimating = false;
  for (int i = 0; i < 75; i++) {
    if (buses[i].active && buses[i].animating) {
      float diff = buses[i].target_led - buses[i].current_led;
      if (abs(diff) < 0.5) {
        buses[i].current_led = buses[i].target_led;
        buses[i].animating = false;
      } else {
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

  for (int i = 0; i < 25; i++) {
    if (buses[i].active) {
      int ledPos = (int)buses[i].current_led;
      if (ledPos >= 0 && ledPos < NUM_LEDS) {
        uint32_t color = getRouteColor(buses[i].route_id);
        strip.setPixelColor(ledPos, color);
      }
    }
  }

  strip.show();
}

// uint32_t getRouteColor(const char* route_id) {
//   if (strcmp(route_id, "TEAL") == 0) return strip.Color(0, 225, 110);
//   if (strcmp(route_id, "GREEN") == 0 || strcmp(route_id, "GREENHOPPER") == 0 || strcmp(route_id, "GREEN EXPRESS") == 0) return strip.Color(0, 255, 0);
//   if (strcmp(route_id, "ILLINI") == 0) return strip.Color(255, 0, 255);
//   if (strcmp(route_id, "YELLOW") == 0 || strcmp(route_id, "YELLOWHOPPER") == 0) return strip.Color(255, 150, 0);
//   if (strcmp(route_id, "RED") == 0) return strip.Color(255, 0, 0);
//   if (strcmp(route_id, "BLUE") == 0) return strip.Color(0, 0, 255);
//   if (strcmp(route_id, "BROWN") == 0) return strip.Color(255, 35, 0);
//   if (strcmp(route_id, "GOLD") == 0 || strcmp(route_id, "GOLDHOPPER") == 0) return strip.Color(255, 85, 0);
//   if (strcmp(route_id, "SILVER") == 0) return strip.Color(200, 200, 200);
//   if (strcmp(route_id, "BRONZE") == 0) return strip.Color(255, 55, 5);
//   if (strcmp(route_id, "RAVEN") == 0) return strip.Color(255, 50, 50);
//   if (strcmp(route_id, "LINK") == 0) return strip.Color(0, 255, 255);
//   return strip.Color(255, 255, 255);
// }

uint32_t getRouteColor(const char* route_id) {

  int index = -1;

  if (strcmp(route_id, "TEAL") == 0) index = 0;
  else if (strcmp(route_id, "GREEN") == 0 || strcmp(route_id, "GREENHOPPER") == 0 || strcmp(route_id, "GREEN EXPRESS") == 0) index = 1;
  else if (strcmp(route_id, "ILLINI") == 0) index = 2;
  else if (strcmp(route_id, "YELLOW") == 0 || strcmp(route_id, "YELLOWHOPPER") == 0) index = 3;
  else if (strcmp(route_id, "RED") == 0) index = 4;
  else if (strcmp(route_id, "BLUE") == 0) index = 5;
  else if (strcmp(route_id, "BROWN") == 0) index = 6;
  else if (strcmp(route_id, "GOLD") == 0 || strcmp(route_id, "GOLDHOPPER") == 0) index = 7;
  else if (strcmp(route_id, "SILVER") == 0) index = 8;
  else if (strcmp(route_id, "BRONZE") == 0) index = 9;
  else if (strcmp(route_id, "RAVEN") == 0) index = 10;
  else if (strcmp(route_id, "LINK") == 0) index = 11;

  // If nothing matched → return white
  if (index == -1) return strip.Color(255, 255, 255);

  // Pull themed RGB from routeColors[]
  RGB c = RGBrouteColors[index];

  return strip.Color(c.r, c.g, c.b);
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
      while (fullRouteID[i] != ' ' && fullRouteID[i] != '\0' && i < sizeof(routeID) - 1) {
        routeID[i] = fullRouteID[i];
        i++;
      }
      routeID[i] = '\0';

      double lat = vehicle["location"]["lat"];
      double lon = vehicle["location"]["lon"];
      const char* direction = vehicle["trip"]["direction"];

      int new_led_position = calculateLEDPosition(lat, lon);

      if (new_led_position >= 0) {
        Serial.print("Route: ");
        Serial.print(routeID);
        Serial.print(" | LED: ");
        Serial.println(new_led_position);

        strcpy(buses[busIndex].route_id, routeID);
        buses[busIndex].lat = lat;
        buses[busIndex].lon = lon;
        buses[busIndex].active = true;
        if (buses[busIndex].current_led < 0) {
          buses[busIndex].current_led = new_led_position;
          buses[busIndex].target_led = new_led_position;
          buses[busIndex].animating = false;
        } else {
          buses[busIndex].target_led = new_led_position;
          buses[busIndex].animating = true;
          busesAnimating = true;
        }

        onRouteCount++;
        busIndex++;
      }
    }

    activeBusCount = onRouteCount;
    Serial.printf("\n=== Summary ===\n");
    Serial.printf("Total buses: %d\n", totalBusesScanned);
    Serial.printf("On tracked routes: %d\n", onRouteCount);
  }
  http.end();
}


void initBrightness() {
  // Initialize smoothing array
  for (int i = 0; i < SMOOTHING_SAMPLES; i++) {
    photoReadings[i] = 0;
  }
}

void cycleBrightness() {
  // Cycle through brightness modes
  currentBrightnessMode = (BrightnessMode)((currentBrightnessMode + 1) % 5);
  
  Serial.print("Brightness mode: ");
  switch(currentBrightnessMode) {
    case BRIGHTNESS_LOW_MODE:
      Serial.println("LOW");
      setBrightnessFixed(BRIGHTNESS_LOW);
      break;
    case BRIGHTNESS_MEDLOW_MODE:
      Serial.println("MEDIUM-LOW");
      setBrightnessFixed(BRIGHTNESS_MEDLOW);
      break;
    case BRIGHTNESS_MED_MODE:
      Serial.println("MEDIUM");
      setBrightnessFixed(BRIGHTNESS_MED);
      break;
    case BRIGHTNESS_HIGH_MODE:
      Serial.println("HIGH");
      setBrightnessFixed(BRIGHTNESS_HIGH);
      break;
    case BRIGHTNESS_AUTO_MODE:
      Serial.println("AUTO (Photoresistor)");
      break;
  }
}

void setBrightnessFixed(int brightness) {
  strip.setBrightness(brightness);
  // menuStrip.setBrightness(brightness);
  strip.show();
  // menuStrip.show();
}

//averages out the readings
int getSmoothedPhotoReading() {
  // Remove oldest reading from total
  photoTotal = photoTotal - photoReadings[photoIndex];
  
  // Read photoresistor
  photoReadings[photoIndex] = analogRead(PHOTORESISTOR_Analog);


  // Add new reading to total
  photoTotal = photoTotal + photoReadings[photoIndex];
  
  // Advance to next position
  photoIndex = (photoIndex + 1) % SMOOTHING_SAMPLES;
  
  // Return average
  return photoTotal / SMOOTHING_SAMPLES;
}
int mapPhotoresistorToBrightness(int photoValue) {
  // ESP32 ADC range: 0-4095
  // LOW ADC = bright room, HIGH ADC = dark room
  // Use ranges to determine brightness level
  
  int brightness;
  
  if (photoValue <= 1500) {
    // Very bright environment (0-1500) -> Max brightness (255)
    // Large range for typical room brightness
    brightness = 255;
  } 
  else if (photoValue <= 2200) {
    // Medium-bright environment (1501-2200) -> Medium-high brightness (150-254)
    brightness = 254 - (((photoValue - 1500) * 104) / 700);
  }
  else if (photoValue <= 2800) {
    // Medium environment (2201-2800) -> Medium brightness (80-149)
    brightness = 149 - (((photoValue - 2200) * 69) / 600);
  }
  else if (photoValue <= 3400) {
    // Dim environment (2801-3400) -> Low brightness (30-79)
    brightness = 79 - (((photoValue - 2800) * 49) / 600);
  }
  else {
    // Very dim environment (3401-4095) -> Minimum brightness (1-29)
    brightness = 29 - (((photoValue - 3400) * 28) / 695);
  }
  
  // Clamp to valid range
  if (brightness < 1) brightness = 1;
  if (brightness > 255) brightness = 255;
  
  // Debug output
  Serial.print("Photo ADC: ");
  Serial.print(photoValue);
  Serial.print(" -> Brightness: ");
  Serial.println(brightness);
  
  return brightness;
}
void updateBrightness() {
  if (currentBrightnessMode == BRIGHTNESS_AUTO_MODE) {
    int photoValue = getSmoothedPhotoReading();
    int brightness = mapPhotoresistorToBrightness(photoValue);
    
    strip.setBrightness(brightness);
    // menuStrip.setBrightness(brightness);
    //strip.show()
  }
}


void cycleThemes() {

    // Increment theme enum
    currentTheme = static_cast<ThemesColor>(currentTheme + 1);
    Serial.println(currentTheme);
    // Wrap around
    if (currentTheme > AMERICA) {
        currentTheme = THEME_DEFAULT;
    }

    // Copy selected theme route colors into active buffer
    for (int i = 0; i < 12; i++) {
        RGBrouteColors[i] = allThemes[currentTheme].routes[i];
    }

    Serial.print("Switched to theme: ");
    Serial.println(currentTheme);
}


