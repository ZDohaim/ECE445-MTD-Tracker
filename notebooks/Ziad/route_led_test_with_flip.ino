#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>

#define LED_PIN     2
#define LED_REDPIN  17
#define LED_GREENPIN 18
#define LED_BLUEPIN 8
#define NUM_LEDS    874  // Total LEDs (0-873)
#define BUTTON_PIN  15
#define BRIGHTNESS  100
#define ANIMATION_DELAY 20  // milliseconds between each LED lighting up

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Button variables
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int currentRoute = 0;
const int NUM_ROUTES = 11;  // Changed from 12 to 11 (removed LINK)

// Route names for Serial output
const char* routeNames[] = {
  "TEAL", "GREEN", "ILLINI", "YELLOW", "RED", "BLUE",
  "BROWN", "GOLD", "SILVER", "BRONZE", "RAVEN"
};

// Route colors (you can customize these)
uint32_t routeColors[] = {
  strip.Color(0, 255, 255),      // TEAL (Cyan)
  strip.Color(0, 255, 0),        // GREEN
  strip.Color(255, 165, 0),      // ILLINI (Orange)
  strip.Color(255, 255, 0),      // YELLOW
  strip.Color(255, 0, 0),        // RED
  strip.Color(0, 0, 255),        // BLUE
  strip.Color(139, 69, 19),      // BROWN
  strip.Color(255, 215, 0),      // GOLD
  strip.Color(192, 192, 192),    // SILVER
  strip.Color(205, 127, 50),     // BRONZE
  strip.Color(128, 0, 128)       // RAVEN (Purple)
};

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();
  
  Serial.println("MTD Bus Route LED Tester");
  Serial.println("Press button to cycle through routes");
  Serial.println("========================");
  
  // Display first route
  displayRoute(currentRoute);
}

void loop() {
  // Read button with debouncing
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == HIGH) {
        // Clear current display
        strip.clear();
        strip.show();
        delay(200);
        
        // Move to next route
        currentRoute = (currentRoute + 1) % NUM_ROUTES;
        
        // Display new route
        displayRoute(currentRoute);
      }
    }
  }
  
  lastButtonState = reading;
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
  }
  
  Serial.println("Route display complete!");
}

void lightSegment(int start, int end, uint32_t color) {
  if (start <= end) {
    for (int i = start; i <= end; i++) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(ANIMATION_DELAY);
    }
  } else {
    // Reverse direction
    for (int i = start; i >= end; i--) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(ANIMATION_DELAY);
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
  
  int gregory1[][2] = {{401, 413}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner");
  
  int goodwin1[][2] = {{426, 442}, {443, 473}, {474, 487}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Gregory-Green");
  
  int green1[][2] = {{800, 835}, {488, 488}, {91, 91}};
  lightMultipleSegments(green1, 3, color, "Green Goodwin-Wright", true);
  
  int wright1[][2] = {{50, 90}};
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  
  int white1[][2] = {{0, 49}};
  lightMultipleSegments(white1, 1, color, "White", true);
}

// GREEN ROUTE
void displayGreen(uint32_t color) {
  int green1[][2] = {{748, 799}, {91, 91}, {800, 835}, {488, 488}};
  lightMultipleSegments(green1, 4, color, "Green Fourth-Goodwin");
  
  int goodwin1[][2] = {{474, 487}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = {{352, 400}, {328, 351}};
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Fourth", true);
  
  int fourth1[][2] = {{298, 307}};
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", true);
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
  lightMultipleSegments(fourth1, 1, color, "Fourth Gregory-End", true);
  
  int daniel1[][2] = {{242, 261}};
  lightMultipleSegments(daniel1, 1, color, "Daniel End-Fourth");
  
  int fourth2[][2] = {{261, 272}, {273, 285}, {173, 173}};
  lightMultipleSegments(fourth2, 3, color, "Fourth Daniel-Armory");
  
  int armory1[][2] = {{138, 149}, {150, 173}};
  lightMultipleSegments(armory1, 2, color, "Armory", true);
  
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
  
  int armory1[][2] = {{138, 149}, {208, 219}, {150, 173}, {174, 184}};
  lightMultipleSegments(armory1, 4, color, "Armory");
  
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
  
  int green1[][2] = {{800, 835}, {488, 488}, {91, 91}};
  lightMultipleSegments(green1, 3, color, "Green Goodwin-Wright", true);
  
  int wright1[][2] = {{50, 90}};
  lightMultipleSegments(wright1, 1, color, "Wright Green-White", true);
  
  int white1[][2] = {{0, 49}};
  lightMultipleSegments(white1, 1, color, "White", true);
}

// BLUE ROUTE
void displayBlue(uint32_t color) {
  int sixth1[][2] = {{232, 241}, {220, 231}, {208, 219}, {174, 184}};
  lightMultipleSegments(sixth1, 4, color, "Sixth End-Armory", true);
  
  int armory1[][2] = {{138, 149}, {351, 351}};
  lightMultipleSegments(armory1, 2, color, "Armory", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}, {50, 90}, {0, 49}};
  lightMultipleSegments(wright1, 4, color, "Wright Armory-White", true);
}

// BROWN ROUTE
void displayBrown(uint32_t color) {
  int sixth1[][2] = {{232, 241}, {220, 231}, {208, 219}};
  lightMultipleSegments(sixth1, 3, color, "Sixth End-Chalmers", true);
  
  int chalmers1[][2] = {{174, 184}};
  lightMultipleSegments(chalmers1, 1, color, "Chalmers Sixth-Wright", true);
  
  int wright1[][2] = {{91, 125}, {50, 90}};
  lightMultipleSegments(wright1, 2, color, "Wright Chalmers-White", true);
  
  int white1[][2] = {{0, 49}, {149, 149}, {351, 351}};
  lightMultipleSegments(white1, 3, color, "White", true);
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
  
  int gregory1[][2] = {{401, 413}, {352, 400}, {174, 184}};
  lightMultipleSegments(gregory1, 3, color, "Gregory Dorner-Sixth", true);
  
  int sixth1[][2] = {{220, 231}, {208, 219}};
  lightMultipleSegments(sixth1, 2, color, "Sixth Gregory-Armory", true);
  
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
  int fourth1[][2] = {{298, 307}, {286, 297}, {327, 327}, {173, 173}};
  lightMultipleSegments(fourth1, 4, color, "Fourth End-Armory", true);
  
  int armory1[][2] = {{150, 173}, {138, 149}};
  lightMultipleSegments(armory1, 2, color, "Armory Fourth-Wright");
  
  int wright1[][2] = {{91, 125}, {126, 137}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green");
  
  int green1[][2] = {{91, 91}, {800, 835}, {488, 488}};
  lightMultipleSegments(green1, 3, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{443, 473}, {474, 487}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = {{401, 413}};
  lightMultipleSegments(gregory1, 1, color, "Gregory Goodwin-Dorner");
  
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}

// RAVEN ROUTE
void displayRaven(uint32_t color) {
  int sixth1[][2] = {{232, 241}, {220, 231}};
  lightMultipleSegments(sixth1, 2, color, "Sixth End-Armory", true);
  
  int armory1[][2] = {{138, 149}};
  lightMultipleSegments(armory1, 1, color, "Armory Sixth-Wright", true);
  
  int wright1[][2] = {{126, 137}, {91, 125}};
  lightMultipleSegments(wright1, 2, color, "Wright Armory-Green");
  
  int green1[][2] = {{800, 835}};
  lightMultipleSegments(green1, 1, color, "Green Wright-Goodwin");
  
  int goodwin1[][2] = {{474, 488}, {443, 473}, {426, 442}};
  lightMultipleSegments(goodwin1, 3, color, "Goodwin Green-Gregory", true);
  
  int gregory1[][2] = {{351, 351}, {400, 413}};
  lightMultipleSegments(gregory1, 2, color, "Gregory Goodwin-Dorner", true);
  
  int dorner1[][2] = {{414, 425}};
  lightMultipleSegments(dorner1, 1, color, "Dorner Gregory-End");
}
