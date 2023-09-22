// Include necessary libraries
#include <Wire.h>                      // Include the Wire library for I2C communication
#include <LiquidCrystal_I2C.h>         // Include the library for the I2C LCD
#include <Adafruit_NeoPixel.h>         // Include the library for controlling NeoPixel LEDs
#include <ESP8266WiFi.h>               // Include the library for the ESP8266 WiFi module
#include <BlynkSimpleEsp8266.h>        // Include Blynk library for ESP8266

// Blynk configurations
#define BLYNK_TEMPLATE_ID "TMPL6-Ys7s4AQ"
#define BLYNK_TEMPLATE_NAME "ULTIMATE GREEN HOUSE"
#define BLYNK_AUTH_TOKEN "VyIuP_UYw2ZNu3AUo6qBZZcMI69XEf3R"
#define BLYNK_PRINT Serial             // Define where Blynk debug messages will be printed

#define NUM_LEDS  16                   // Define the number of LEDs

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;       // Blynk authentication token
char ssid[] = "Beg";                  // WiFi SSID
char pass[] = "26122004";             // WiFi password

// Pin configurations
const int lightSensorPin = D1;        // Digital pin for light sensor
const int waterLevelSensorPin = A0;   // Analog pin for water level sensor
const int ledPin = D3;                // Digital pin for the LED strip

// Global variables
int soilMoistureValue = 0;            // Store the soil moisture value
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, ledPin, NEO_GRB + NEO_KHZ800);
bool waterNotificationSent = false;   // To prevent sending multiple notifications

// Blynk callback: updates when virtual pin V3 receives data
BLYNK_WRITE(V3) {
  soilMoistureValue = param.asInt();  // Store the soil moisture value
}

void setup() {
  Serial.begin(9600);                // Initialize serial communication at 9600 baud
  Blynk.begin(auth, ssid, pass);     // Connect to Blynk with auth token and WiFi credentials
  strip.begin();                     // Initialize the NeoPixel strip
  strip.show();                      // Set all pixels in the strip to 'off'
  randomSeed(analogRead(0));        // Seed the random number generator for random colors
  pinMode(ledPin, OUTPUT);           // Set the LED pin as output
}

// Function to blink the LED strip with random colors
void blinkWithRandomColors(int duration) {
  int numPixels = strip.numPixels();

  for (int j = 0; j < numPixels * 2; j++) {
    // Generate random colors for each LED
    for (int i = 0; i < numPixels; i++) {
      uint8_t r = random(256);
      uint8_t g = random(256);
      uint8_t b = random(256);
      uint32_t color = strip.Color(r, g, b);
      strip.setPixelColor(i, color);
    }
    
    strip.show();
    delay(duration / 10);  // On for half of the duration
    
  }
}

void loop() {
  Blynk.run();                       // Run Blynk background tasks

  // Read sensor values
  int waterLevelValue = analogRead(waterLevelSensorPin);
  int lightValue = digitalRead(lightSensorPin);

  // Calculate water level percentage
  int waterLevelPercentage = map(waterLevelValue, 0, 520, 0, 100);
  
  // Send water level percentage to Blynk
  Blynk.virtualWrite(V8, waterLevelPercentage);

  // Request the latest value of V3 (soil moisture) from the Blynk server
  Blynk.syncVirtual(V3);

  // Display sensor data on the Serial Monitor
  Serial.print("Water Level Percentage: ");
  Serial.println(waterLevelPercentage);
  Serial.print("Light Value: ");
  Serial.println(lightValue);

  // Blynk notification for low water level
  if (waterLevelPercentage < 20 && !waterNotificationSent) {
    Blynk.logEvent("waterlevel", "Water level is below 20%!");
    waterNotificationSent = true;
  } else if (waterLevelPercentage >= 20) {
    waterNotificationSent = false;
  }

  // Control LED based on light value
  if (lightValue == 0) {
    // strip.clear();
    strip.fill(strip.Color(0, 0, 0));
    strip.show();
    Blynk.virtualWrite(V10, "Day");  // Assuming V10 is a labeled display widget in Blynk
  } else if (lightValue == 1 && soilMoistureValue < 30) {
    blinkWithRandomColors(500);
    Blynk.virtualWrite(V10, "Night");
  } else if (lightValue == 1 && soilMoistureValue > 30) {
    strip.fill(strip.Color(255, 255, 255));
    strip.show();
    Blynk.virtualWrite(V10, "Night");
  }

  delay(1000);
}

