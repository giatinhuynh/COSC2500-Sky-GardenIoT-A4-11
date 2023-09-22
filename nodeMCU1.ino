// Sky Garden - Advanced Automated Greenhouse System

// Define connections for various components
/* Connections
Relay (Water Pump) - D3
Light Level Sensor - D7
Soil Moisture Sensor - A0
PIR Motion Sensor - D5
SDA for LCD - D2
SCL for LCD - D1
DHT11 Temperature and Humidity Sensor - D4
Fan - D6
*/

// Include required libraries and set up Blynk configurations
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#define BLYNK_TEMPLATE_ID "TMPL6-Ys7s4AQ"
#define BLYNK_TEMPLATE_NAME "ULTIMATE GREEN HOUSE"
#define BLYNK_AUTH_TOKEN "VyIuP_UYw2ZNu3AUo6qBZZcMI69XEf3R"
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define BLYNK_PRINT Serial  // For Blynk debugging

// Initialize the LCD with I2C address and dimensions
LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Beg";
char pass[] = "26122004";

// Initialize DHT sensor
DHT dht(D4, DHT11);
BlynkTimer timer;

// Define pin numbers for components
#define soil A0
#define PIR D5
#define RELAY_PIN_1 D3
#define LIGHT_LEVEL_SENSOR D7
#define FAN_RELAY D6

// Define initial state variables
int PIR_ToggleValue;
int relay1State = LOW; // Water pump initial state
int fanState = LOW;    // Fan initial state

void setup() {
  // Begin serial communication for debugging
  Serial.begin(9600);
  
  // Initialize the LCD and turn on its backlight
  lcd.init();
  lcd.backlight();
  
  // Set pin modes for inputs and outputs
  pinMode(PIR, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(LIGHT_LEVEL_SENSOR, INPUT);
  pinMode(FAN_RELAY, OUTPUT);

  // Set initial states for relays
  digitalWrite(RELAY_PIN_1, relay1State);
  digitalWrite(FAN_RELAY, fanState);

  // Connect to Blynk and initialize DHT sensor
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  // Display initialization message on LCD
  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(11, 1);
  lcd.print("W:OFF");

  // Set timer intervals for reading sensors
  timer.setInterval(100L, soilMoistureSensor);
  timer.setInterval(100L, DHT11sensor);
}

// Read DHT11 sensor and send temperature and humidity to Blynk
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Send data to Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  // Display temperature and humidity on LCD
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);

  // Control fan based on temperature
  if (t > 30) {
    fanState = HIGH;
  } else {
    fanState = LOW;
  }
  digitalWrite(FAN_RELAY, fanState);
  Blynk.virtualWrite(V7, fanState);
}

// Read soil moisture sensor and send data to Blynk
void soilMoistureSensor() {
  int value = analogRead(soil);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  Blynk.virtualWrite(V3, value);
  
  // Display soil moisture on LCD
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);

  // Control water pump based on soil moisture
  if (value < 30) {
    relay1State = HIGH;
  } else {
    relay1State = LOW;
  }
  digitalWrite(RELAY_PIN_1, relay1State);
  Blynk.virtualWrite(V12, relay1State);
}

// Detect motion using PIR sensor and log it in Blynk
void PIRsensor() {
  bool value = digitalRead(PIR);
  if (value) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");
  }
}

// Handle changes from Blynk app for PIR sensor
BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();
}

// Handle fan state changes from Blynk app
BLYNK_WRITE(V7) {
  fanState = param.asInt();
  digitalWrite(FAN_RELAY, fanState);
}

// Handle water pump state changes from Blynk app
BLYNK_WRITE(V12) {
  relay1State = param.asInt();
  digitalWrite(RELAY_PIN_1, relay1State);
}

void loop() {
  // Display fan state on LCD
  if (fanState == HIGH) {
    lcd.setCursor(5, 1);
    lcd.print("F:ON ");
  } else {
    lcd.setCursor(5, 1);
    lcd.print("F:OFF");
  }

  // Display water pump state on LCD
  if (relay1State == HIGH) {
    lcd.setCursor(11, 1);
    lcd.print("W:ON ");
  } else {
    lcd.setCursor(11, 1);
    lcd.print("W:OFF");
  }

  // Read light level and send to Blynk
  int lightLevel = analogRead(LIGHT_LEVEL_SENSOR);
  Blynk.virtualWrite(V9, lightLevel);

  // Run Blynk and timer processes
  Blynk.run();
  timer.run();
}
