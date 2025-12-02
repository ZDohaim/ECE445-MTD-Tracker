#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>

#define LED_PIN     2
#define LED_STRIP_PIN 1
#define LED_REDPIN  17
#define LED_GREENPIN 18
#define LED_BLUEPIN 8
#define NUM_LEDS    874  // Total LEDs (0-873)
#define MENU_LEDS   12   // 12 LEDs for menu strip
#define BUTTON_PIN  15
#define BRIGHTNESS  100
#define MENU_TIMEOUT 10000  // 10 seconds timeout
#define FLASH_INTERVAL 500  // Flash interval for menu mode (ms)
#define HOLD_THRESHOLD 2000 // 1 second hold

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel menuStrip(MENU_LEDS, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Button variables
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long buttonPressStart = 0;
unsigned long lastMenuActivity = 0;
unsigned long lastFlashTime = 0;
bool buttonHeld = false;
bool flashState = false;

// Mode variables
enum Mode {
  DISPLAY_MODE,
  MENU_MODE
};

Mode currentMode = DISPLAY_MODE;
int currentRoute = 0;
int menuSelection = 0;
const int NUM_ROUTES = 12;

// Route names for Serial output
const char* routeNames[] = {
  "TEAL", "GREEN", "ILLINI", "YELLOW", "RED", "BLUE",
  "BROWN", "GOLD", "SILVER", "BRONZE", "RAVEN", "LINK"
};

// Route colors 
uint32_t routeColors[] = {
  menuStrip.Color(0, 255, 255),      // TEAL (Cyan)
  menuStrip.Color(0, 255, 0),        // GREEN
  menuStrip.Color(255, 165, 0),      // ILLINI (Orange)
  menuStrip.Color(255, 255, 0),      // YELLOW
  menuStrip.Color(255, 0, 0),        // RED
  menuStrip.Color(0, 0, 255),        // BLUE
  menuStrip.Color(139, 69, 19),      // BROWN
  menuStrip.Color(255, 215, 0),      // GOLD
  menuStrip.Color(192, 192, 192),    // SILVER
  menuStrip.Color(205, 127, 50),     // BRONZE
  menuStrip.Color(128, 0, 128),      // RAVEN (Purple)
  menuStrip.Color(255, 255, 255)     // LINK (White)
};

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();
  
  menuStrip.begin();
  menuStrip.setBrightness(BRIGHTNESS);
  
  // Initialize menu strip - show all route colors as indicator
  showAllRoutes();
  
  Serial.println("MTD Bus Route LED Tester");
  Serial.println("Hold button for 1 second to enter menu mode");
  Serial.println("========================");
}

void loop() {
  // Read button with debouncing
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    Serial.print("Button state changed. Reading: ");
    Serial.println(reading == LOW ? "LOW (pressed)" : "HIGH (released)");
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed (going LOW)
      if (buttonState == LOW) {
        buttonPressStart = millis();
        buttonHeld = false;
        Serial.print("Button press START at: ");
        Serial.println(buttonPressStart);
      }
      // Button released (going HIGH)
      else {
        unsigned long releaseTime = millis();
        unsigned long pressDuration = releaseTime - buttonPressStart;
        
        Serial.println("========================================");
        Serial.print("Button RELEASED at: ");
        Serial.println(releaseTime);
        Serial.print("Press started at: ");
        Serial.println(buttonPressStart);
        Serial.print("Press duration: ");
        Serial.print(pressDuration);
        Serial.println(" ms");
        Serial.print("Hold threshold: ");
        Serial.print(HOLD_THRESHOLD);
        Serial.println(" ms");
        
        if (pressDuration >= HOLD_THRESHOLD) {
          // Was a hold
          Serial.println(">>> HOLD DETECTED <<<");
          
          if (currentMode == DISPLAY_MODE) {
            Serial.println("Action: Entering menu mode");
            enterMenuMode();
          }
          else if (currentMode == MENU_MODE) {
            Serial.println("Action: Confirming selection");
            confirmSelection();
          }
        } else {
          // Was a quick press
          Serial.println(">>> QUICK PRESS DETECTED <<<");
          
          if (currentMode == MENU_MODE) {
            Serial.println("Action: Next menu selection");
            nextMenuSelection();
          } else {
            Serial.println("Action: Nothing (in display mode)");
          }
        }
        Serial.println("========================================\n");
      }
    }
  }
  
  lastButtonState = reading;
  
  // Handle menu mode
  if (currentMode == MENU_MODE) {
    // Check for timeout
    if (millis() - lastMenuActivity > MENU_TIMEOUT) {
      exitMenuMode();
    }
    
    // Handle flashing
    if (millis() - lastFlashTime > FLASH_INTERVAL) {
      lastFlashTime = millis();
      flashState = !flashState;
      updateMenuDisplay();
    }
  }
}
void enterMenuMode() {
  currentMode = MENU_MODE;
  menuSelection = 0;
  lastMenuActivity = millis();
  lastFlashTime = millis();
  flashState = true;
  
  
  // Clear main strip
  strip.clear();
  strip.show();
  
  updateMenuDisplay();
}

void exitMenuMode() {
  currentMode = DISPLAY_MODE;
  
  Serial.println("\n>>> EXITING MENU MODE (timeout) <<<");
  Serial.println("Returning to display mode");
  
  // Restore menu strip to show all routes
  showAllRoutes();
  
  // Clear main strip
  strip.clear();
  strip.show();
}

void nextMenuSelection() {
  menuSelection = (menuSelection + 1) % NUM_ROUTES;
  lastMenuActivity = millis();
  flashState = true;
  lastFlashTime = millis();
  
  Serial.print("Menu selection: ");
  Serial.println(routeNames[menuSelection]);
  
  updateMenuDisplay();
}

void confirmSelection() {
  currentRoute = menuSelection;
  
  Serial.println("\n>>> SELECTION CONFIRMED <<<");
  Serial.print("Displaying route: ");
  Serial.println(routeNames[currentRoute]);
  
  // Exit menu mode
  currentMode = DISPLAY_MODE;
  
  // Restore menu strip to show all routes
  showAllRoutes();
  
  // Display the selected route on main strip
  displayRoute(currentRoute);
}

void updateMenuDisplay() {
  // Turn off all menu LEDs
  menuStrip.clear();
  
  // Only flash the selected LED
  if (flashState) {
    menuStrip.setPixelColor(menuSelection, routeColors[menuSelection]);
  }
  // When flashState is false, LED stays off (already cleared)
  
  menuStrip.show();
}

void showAllRoutes() {
  // Display all route colors on menu strip as indicator
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
  
  switch(routeIndex) {
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
  Serial.print("  - ");
  Serial.println(segmentName);
  
  for (int i = 0; i < numSegments; i++) {
    if (flip) {
      // Reverse the segment direction
      lightSegment(segments[i][1], segments[i][0], color);
    } else {
      lightSegment(segments[i][0], segments[i][1], color);
    }
  }
}

// TEAL ROUTE
void displayTeal(uint32_t color) {
  int dorner[][2] = {{414, 425}};
  lightMultipleSegments(dorner, 1, color, "Dorner Gregory-End", true);
  
  int gregory1[][2] = {{400, 415}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", true);
  
  int goodwin1[][2] = {{426, 442}, {443, 473}, {474, 488}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Gregory-Green");
  
  int green1[][2] = {{800, 835}};
  lightMultipleSegments(green1, 1, color, "Green Goodwin-Wright", true);
  
  int wright1[][2] = {{50, 91}};
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  
  int white1[][2] = {{0, 49}};
  lightMultipleSegments(white1, 1, color, "White", true);
}

// GREEN ROUTE
void displayGreen(uint32_t color) {
  int green1[][2] = {{748, 799}, {91, 91}, {800, 835}, {488, 488}, {836, 873}};
  lightMultipleSegments(green1, 5, color, "Green Fourth-Goodwin");
  
  int goodwin1[][2] = {{474, 487}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = {{352, 400}, {327, 351}};
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Fourth", true);
  
  int fourth1[][2] = {{298, 307}};
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", false);
}

// ILLINI ROUTE
void displayIllini(uint32_t color) {
  int lincoln1[][2] = {{684, 713}, {654, 683}};
  lightMultipleSegments(lincoln1, 2, color, "Lincoln End-Illinois", true);
  
  int illinois1[][2] = {{620, 653}};
  lightMultipleSegments(illinois1, 1, color, "Illinois End-Goodwin", true);
  
  int goodwin1[][2] = {{443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 2, color, "Goodwin Illinois-Gregory", true);
  
  int gregory1[][2] = {{352, 400}, {327, 351}};
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Fourth", true);
  
  int fourth1[][2] = {{298, 307}};
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", false);
  
  int daniel1[][2] = {{242, 261}};
  lightMultipleSegments(daniel1, 1, color, "Daniel End-Fourth");
  
  int fourth2[][2] = {{261, 272}, {273, 285}};
  lightMultipleSegments(fourth2, 2, color, "Fourth Daniel-Armory");
  
  int armory1[][2] = { {173, 150}, {149, 138}};
  lightMultipleSegments(armory1, 2, color, "Armory", false);
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  
  int green1[][2] = {{91, 91}, {800, 835}, {488, 488}};
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  
  int goodwin2[][2] = {{488, 510}, {511, 545}};
  lightMultipleSegments(goodwin2, 2, color, "Goodwin Green-University");
  
  int university1[][2] = {{546, 581}};
  lightMultipleSegments(university1, 1, color, "University Goodwin-Lincoln");
}

// YELLOW ROUTE
void displayYellow(uint32_t color) {
  int gregory1[][2] = {{308, 327}};
  lightMultipleSegments(gregory1, 1, color, "Gregory End-Fourth");
  
  int fourth1[][2] = {{286, 297}, {327, 327}, {173, 173}};
  lightMultipleSegments(fourth1, 3, color, "Fourth Gregory-Armory", true);
  
  int armory1[][2] = {{150, 173},{138, 149}, {208, 219}, {174, 184}};
  lightMultipleSegments(armory1, 4, color, "Armory", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}, {50, 90}};
  lightMultipleSegments(wright1, 3, color, "Wright Armory-White", true);
  
  int white1[][2] = {{0, 49}};
  lightMultipleSegments(white1, 1, color, "White", true);
}

// RED ROUTE
void displayRed(uint32_t color) {
  int lincoln1[][2] = {{684, 713}};
  lightMultipleSegments(lincoln1, 1, color, "Lincoln End-Nevada", true);
  
  int nevada1[][2] = {{714, 747}};
  lightMultipleSegments(nevada1, 1, color, "Nevada Lincoln-Goodwin");
  
  int goodwin1[][2] = {{443, 473}, {474, 487}};
  lightMultipleSegments(goodwin1, 2, color, "Goodwin Nevada-Green");
  
  int green1[][2] = {{488, 488}, {800, 835} };
  lightMultipleSegments(green1, 2, color, "Green Goodwin-Wright", true);
  
  int wright1[][2] = {{50, 91}};
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  
  int white1[][2] = {{0, 49}};
  lightMultipleSegments(white1, 1, color, "White", true);
}

// BLUE ROUTE
void displayBlue(uint32_t color) {
  int sixth1[][2] = { {232, 241},{351,351}, {220, 231},{149, 149}, {208, 219}, {174, 184}};
  lightMultipleSegments(sixth1, 6, color, "Sixth End-Armory", true);
  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}, {50, 90}, {0, 49}};
  lightMultipleSegments(wright1, 4, color, "Wright Armory-White", true);
}

// BROWN ROUTE
void displayBrown(uint32_t color) {
  int sixth1[][2] = { {232, 241}, {351,351}, {220, 231}, {149, 149}, {208, 219}, {174, 184}};
  lightMultipleSegments(sixth1, 6, color, "Sixth End-Armory", true);
  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}, {50, 90}, {0, 49}};
  lightMultipleSegments(wright1, 4, color, "Wright Armory-White", true);
}

// GOLD ROUTE
void displayGold(uint32_t color) {
  int gregory1[][2] = {{308, 327}, {328, 351}, {352, 400}};
  lightMultipleSegments(gregory1, 3, color, "Gregory End-Goodwin");
  
  int goodwin1[][2] = {{426, 442}, {443, 473}, {474, 487}, {488, 510}};
  lightMultipleSegments(goodwin1, 4, color, "Goodwin Gregory-Springfield");
  
  int springfield1[][2] = {{582, 619}};
  lightMultipleSegments(springfield1, 1, color, "Springfield Goodwin-End", true);
}

// SILVER ROUTE
void displaySilver(uint32_t color) {
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End", true);
  
  int gregory1[][2] = {{401, 413}, {351, 400}};
  lightMultipleSegments(gregory1, 2, color, "Gregory Dorner-Sixth", true);
  
  int sixth1[][2] = {{220, 231}, {208, 219}, {174, 184}};
  lightMultipleSegments(sixth1, 3, color, "Sixth Gregory-Armory", true);
  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  
  int green1[][2] = {{91, 91}, {800, 835}, {488, 488}};
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{488, 510}};
  lightMultipleSegments(goodwin1, 1, color, "Goodwin Green-Springfield");
  
  int springfield1[][2] = {{582, 619}};
  lightMultipleSegments(springfield1, 1, color, "Springfield Goodwin-End", true);
}

// BRONZE ROUTE
void displayBronze(uint32_t color) {
  int fourth1[][2] = { {298, 307}, {327, 327}, {286, 297}};
  lightMultipleSegments(fourth1, 3, color, "Fourth End-Armory", true);
  
  int armory1[][2] = {{150, 173}, {138, 149}};
  lightMultipleSegments(armory1, 2, color, "Armory Fourth-Wright",true);
  
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);

  
  int green1[][2] = {{800, 835}};
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{474, 488}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = { {400, 413}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", false);
  
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}

// RAVEN ROUTE
void displayRaven(uint32_t color) {
  int sixth1[][2] = {{232, 241}, {351, 351},{220, 231}};
  lightMultipleSegments(sixth1, 3, color, "Sixth End-Armory", true);

  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  
  int green1[][2] = {{800, 835}};
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{474, 488}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = { {400, 413}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", false);
  
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}

// LINK ROUTE
void displayLink(uint32_t color) {
  int sixth1[][2] = {{232, 241}, {351, 351},{220, 231}};
  lightMultipleSegments(sixth1, 3, color, "Sixth End-Armory", true);

  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green", true);
  
  int green1[][2] = {{800, 835}};
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{474, 488}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = { {400, 413}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner", false);
  
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}