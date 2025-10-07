// #include <Wire.h>
// #include <Adafruit_SSD1306.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_NeoPixel.h>

// // Pin definitions
// #define PHOTORESISTOR_PIN 1    // Analog pin for photoresistor A0
// #define BUTTON1_PIN 4          // Digital input pin for Button 1
// #define BUTTON2_PIN 5          // Digital input pin for Button 2
// #define LED_PIN 36              // Digital output for a single LED
// #define NEOPIXEL_PIN 16         // Pin for NeoPixel LED strip
// #define OLED_SDA_PIN 39         // I2C SDA pin
// #define OLED_SCL_PIN 38         // I2C SCL pin
// #define SSD1306_I2C_ADDRESS 0x3C

// // LED strip setup
// #define NUM_LEDS 160            // Number of LEDs in the NeoPixel strip
// Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// // OLED setup
// #define SCREEN_WIDTH 128       // OLED display width, in pixels
// #define SCREEN_HEIGHT 64       // OLED display height, in pixels
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Use hardware I2C

// int intensity = 255;

// void setup() {
//   // Initialize Serial Monitor
//   Serial.begin(115200);

//   // Initialize OLED display
//   Serial.println("Initilizing Display");
//   Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
//     Serial.println("debug2");
//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;);
//   }
//   display.display();
//   Serial.println("debug3");
//   delay(100);  // Pause for 2 seconds

//   // Initialize NeoPixel LED strip
//   strip.begin();
//   strip.show(); // Initialize all pixels to 'off'
//   //strip.setBrightness(64);  // Set brightness to 50%
//   strip.clear();
//   strip.show();

//   // Initialize the LED (for output)
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, LOW);  // Turn off initially

//   // Initialize buttons (inputs)
//   pinMode(BUTTON1_PIN, INPUT_PULLUP);
//   pinMode(BUTTON2_PIN, INPUT_PULLUP);

//   // Initialize photoresistor (input)
//   pinMode(PHOTORESISTOR_PIN, INPUT);
  
//   // Display a simple message on OLED
//   display.clearDisplay();
//   display.setTextSize(1);      
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0,0);
//   display.print("Initialization Done");
//   display.display();
//   delay(100);  // Pause for 2 seconds\


// }

// void loop() {
//   // Read the photoresistor value (analog input)
//   int lightValue = analogRead(PHOTORESISTOR_PIN);
//   Serial.print("Light Level: ");
//   Serial.println(lightValue);

//   // Map the light value (0 to 4095) to the range (0 to 255) for color intensity
//   int mappedValue = map(lightValue, 0, 4095, 0, 255);

//   // Smooth color transition:
//   // Red will increase as the light level increases
//   int red = mappedValue;

//   // Green will change as a secondary value (e.g., inversely proportional to Red)
//   int green = 255 - mappedValue;

//   // Blue will increase inversely with Green
//   int blue = mappedValue / 2;

//   // Read button states
//   int button1State = digitalRead(BUTTON1_PIN);
//   int button2State = digitalRead(BUTTON2_PIN);
//   Serial.print("Button 1: ");
//   Serial.println(button1State == LOW ? "Pressed" : "Not Pressed");
//   Serial.print("Button 2: ");
//   Serial.println(button2State == LOW ? "Pressed" : "Not Pressed");

  
//   // Turn on LED if button1 is pressed
//   if (button1State == LOW) {
//     digitalWrite(LED_PIN, HIGH);  // Turn LED on
//     if(intensity == 255){
//       intensity = 32;
    
//     }else if(intensity == 32){
//       intensity = 255;
//     }
//       strip.setBrightness(intensity);
//       strip.show();
//       delay(200);
//   } else {
//     digitalWrite(LED_PIN, LOW);   // Turn LED off
//   }


//   if (button2State == LOW) {
//     // If Button 2 is pressed, set all LEDs to the current color for 2 seconds
//     for (int i = 0; i < NUM_LEDS; i++) {
//       strip.setPixelColor(i, strip.Color(red, green, blue));  // Set LED to the current color
//     }
//     strip.show();  // Update the strip
//     delay(2000);    // Wait for 2 seconds

//     // After 2 seconds, clear the LEDs and return to the chasing effect
//     strip.clear();
//     strip.show();
//   } else {
//     // Chasing LED animation with smooth color transition
//     static int currentLED = 0;

//     // Turn off the previous LED (if not the first)
//     strip.setPixelColor(currentLED, strip.Color(0, 0, 0));  // Clear previous LED

//     // Move to the next LED
//     currentLED++;  
//     if (currentLED >= NUM_LEDS) {
//       currentLED = 0;  // Loop back to start
//     }

//     // Set the current LED to the dynamically calculated color based on light value
//     strip.setPixelColor(currentLED, strip.Color(red, green, blue));

//     // Show the updated strip
//     strip.show();

//     delay(100);  // Adjust speed of movement
//   }

//   // Display the photoresistor value on the OLED
//   display.clearDisplay();
//   display.setCursor(0, 0);
//   display.print("Light Value: ");
//   display.println(lightValue);
//   display.display();

//   delay(200);  // Small delay for stability
// }




// // void loop() {
// //   // Read the photoresistor value (analog input)
// //   int lightValue = analogRead(PHOTORESISTOR_PIN);
// //   Serial.print("Light Level: ");
// //   Serial.println(lightValue);

// //   int brightnessFactor = map(lightValue, 0, 4095, 0, 255);  // Lower light value => dimmer LED
// //   // Set RGB values for a custom color and apply brightness factor
// //   int red = 255 * brightnessFactor / 255;   // Red channel
// //   int green = 128 * brightnessFactor / 255; // Green channel (lower intensity)
// //   int blue = 0;  // No blue for now

// //   // Read button states
// //   int button1State = digitalRead(BUTTON1_PIN);
// //   int button2State = digitalRead(BUTTON2_PIN);
// //   Serial.print("Button 1: ");
// //   Serial.println(button1State == LOW ? "Pressed" : "Not Pressed");
// //   Serial.print("Button 2: ");
// //   Serial.println(button2State == LOW ? "Pressed" : "Not Pressed");

// //   // Turn on LED if button1 is pressed
// //   if (button1State == LOW) {
// //     digitalWrite(LED_PIN, HIGH);  // Turn LED on
// //   } else {
// //     digitalWrite(LED_PIN, LOW);   // Turn LED off
// //   }

// //   if (button2State == LOW) {
// //     // If Button 1 is pressed, set all LEDs to yellow for 2 seconds
// //     for (int i = 0; i < NUM_LEDS; i++) {
// //       strip.setPixelColor(i, strip.Color(red, green, blue));  // Yellow
// //     }
// //     strip.show();  // Update the strip
// //     delay(2000);    // Wait for 2 seconds

// //     // After 2 seconds, clear the LEDs and return to chasing effect
// //     strip.clear();
// //     strip.show();
// //   } else {
// //     // Chasing yellow LED animation (no flashing)
// //     static int currentLED = 0;

// //     // Turn off the previous LED (if not the first)
// //     strip.setPixelColor(currentLED, strip.Color(0, 0, 0));  // Clear previous LED

// //     // Move to the next LED
// //     currentLED++;  
// //     if (currentLED >= NUM_LEDS) {
// //       currentLED = 0;  // Loop back to start
// //     }

// //     // Set the current LED to yellow
// //     strip.setPixelColor(currentLED, strip.Color(255, 255, 0));  // Yellow

// //     // Show the updated strip
// //     strip.show();

// //     delay(100);  // Adjust speed of movement
// //   }

// //   // Display the photoresistor value on the OLED
// //   display.clearDisplay();
// //   display.setCursor(0, 0);
// //   display.print("Light Value: ");
// //   display.println(lightValue);
// //   display.display();

// //   delay(200);  // Small delay for stability
// // }



#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>

// Pin definitions
#define PHOTORESISTOR_PIN 1    // Analog pin for photoresistor
#define BUTTON1_PIN 4          // Digital input pin for Button 1
#define BUTTON2_PIN 5          // Digital input pin for Button 2
#define LED_PIN 36             // Digital output for a single LED
#define NEOPIXEL_PIN 16        // Pin for NeoPixel LED strip
#define OLED_SDA_PIN 39        // I2C SDA pin
#define OLED_SCL_PIN 38        // I2C SCL pin
#define SSD1306_I2C_ADDRESS 0x3C

// LED strip setup
#define NUM_LEDS 160           // Number of LEDs in the NeoPixel strip
Adafruit_NeoPixel strip(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// OLED setup
#define SCREEN_WIDTH 128       // OLED display width, in pixels
#define SCREEN_HEIGHT 64       // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Use hardware I2C

// Variables for intensity and color cycling
int intensity = 255;  // Default brightness (intensity)
int currentColorIndex = 0;  // Current color index for Button 2

// Colors for rainbow cycle
uint32_t rainbowColors[] = {
  strip.Color(255, 0, 0),     // Red
  strip.Color(255, 127, 0),   // Orange
  strip.Color(255, 255, 0),   // Yellow
  strip.Color(0, 255, 0),     // Green
  strip.Color(0, 0, 255),     // Blue
  strip.Color(75, 0, 130),    // Indigo
  strip.Color(148, 0, 211)    // Violet
};

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(100);  // Pause for 2 seconds

  // Initialize NeoPixel LED strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
  
  // Set initial brightness
  strip.setBrightness(intensity);
  strip.show();

  // Initialize the LED (for output)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Turn off initially

  // Initialize buttons (inputs)
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // Initialize photoresistor (input)
  pinMode(PHOTORESISTOR_PIN, INPUT);
  
  // Display initial message on OLED
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Initialization Done");
  display.display();
  delay(100);
}

void loop() {
  // Read the photoresistor value (analog input) to control brightness
  int lightValue = analogRead(PHOTORESISTOR_PIN);
  intensity = map(lightValue, 0, 4095, 0, 255);  // Map light value to brightness level
  strip.setBrightness(intensity);  // Update NeoPixel brightness based on light level
  strip.show();  // Update the strip with new brightness

  // Read the button states
  int button1State = digitalRead(BUTTON1_PIN);
  int button2State = digitalRead(BUTTON2_PIN);

  // Turn on LED if Button 1 is pressed (turns all LEDs on with a specific color)
  if (button1State == LOW) {
    // Set all LEDs to the current color (based on the currentColorIndex)
    uint32_t color = rainbowColors[currentColorIndex];
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, color);  // Set LED to the current color
    }
    strip.show();  // Update the strip
    delay(200);  // Debounce delay
  }

  // Button 2 cycles through rainbow colors
  if (button2State == LOW) {
    currentColorIndex++;
    if (currentColorIndex >= sizeof(rainbowColors) / sizeof(rainbowColors[0])) {
      currentColorIndex = 0;  // Loop back to the first color
    }
    delay(200);  // Debounce delay
  }

  // Chasing effect (single LED moves across the strip with the current color)
  static int currentLED = 0;

  // Turn off the previous LED (if not the first)
  strip.setPixelColor(currentLED, strip.Color(0, 0, 0));  // Clear previous LED

  // Move to the next LED
  currentLED++;  
  if (currentLED >= NUM_LEDS) {
    currentLED = 0;  // Loop back to the start
  }

  // Set the current LED to the dynamically calculated color based on the photoresistor
  uint32_t color = rainbowColors[currentColorIndex];
  strip.setPixelColor(currentLED, color);  // Set current LED to the color

  // Show the updated strip
  strip.show();

  delay(100);  // Adjust speed of chasing effect

  // Optional: Display the photoresistor value on the OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Light Value: ");
  display.println(lightValue);
  display.display();

  delay(200);  // Small delay for stability
}

