#include <DHT.h>

// Define pins
const int reedSwitchPin = 27; // GPIO pin connected to the magnetic switch

// DHT11 sensor configuration
#define DHTPIN_KITCHEN 32  // GPIO pin connected to the DHT11 sensor in the kitchen
#define DHTPIN_BATHROOM 33 // GPIO pin connected to the DHT11 sensor in the bathroom
#define DHTTYPE DHT11     // DHT 11

DHT dhtKitchen(DHTPIN_KITCHEN, DHTTYPE);
DHT dhtBathroom(DHTPIN_BATHROOM, DHTTYPE);

// MQ2 sensor pin
const int gasSensorPin = 34; // GPIO pin connected to MQ2 sensor
int gasValue = 0;

// Flame sensor pins
const int FlameanalogInPin = 35;  // A0 attach to pin 35
const int FlamedigitalInPin = 25; // D0 attach to pin 25

// Define the pin numbers for the push buttons
const int buttonPinKitchen = 15;
const int buttonPinBathroom = 14;
const int buttonPinBedroom = 12;
const int buttonPinLivingRoom = 18;

// Rain sensor pin
const int rainSensorPin = 26; // Analog pin for rain sensor

// Touch sensor pin for fall detection in bathroom
const int touchPinBathroom = 19; // GPIO pin connected to the touch sensor

// Status flags
int kitchenHighTemp = 0;
int kitchenHighHumidity = 0;
int bathroomHighTemp = 0;
int bathroomHighHumidity = 0;
int gasAlert = 0;
int flameAlert = 0;
int rainAlert = 0; // Rain sensor alert
int fallAlert = 0; // Fall detection alert

void setup() {
  Serial.begin(115200); // Start the serial communication at a baud rate of 115200
  dhtKitchen.begin();   // Initialize the DHT11 sensor in the kitchen
  dhtBathroom.begin();  // Initialize the DHT11 sensor in the bathroom
  pinMode(FlamedigitalInPin, INPUT);
  pinMode(reedSwitchPin, INPUT_PULLUP); // Set the reed switch pin as input with internal pull-up resistor
  pinMode(buttonPinKitchen, INPUT_PULLUP);
  pinMode(buttonPinBathroom, INPUT_PULLUP);
  pinMode(buttonPinBedroom, INPUT_PULLUP);
  pinMode(buttonPinLivingRoom, INPUT_PULLUP);
  pinMode(touchPinBathroom, INPUT); // Set the touch sensor pin as input
}

void loop() {
  monitorKitchen();
  monitorBathroom();
  monitorLivingRoom();
  monitorBedroom();
  delay(2000); // Wait for 2 seconds before next reading
}

void monitorKitchen() {
  int kitchenButtonState = digitalRead(buttonPinKitchen);
  // Check if the kitchen button is pressed
  if (kitchenButtonState == LOW) { // Assuming LOW means the button is pressed
    Serial.println("Alert: Call for help from the kitchen!");
  }
  // Monitor DHT11 sensor
  float humidity = dhtKitchen.readHumidity();
  float temperature = dhtKitchen.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from kitchen DHT sensor!");
    return;
  }

  Serial.print("Kitchen Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Kitchen Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Check for high temperature in kitchen
  if (temperature > 35.0) {
    kitchenHighTemp = 1;
    Serial.println("Alert: High temperature detected in the kitchen. Please check the area.");
  } else {
    kitchenHighTemp = 0;
  }

  // Check for high humidity in kitchen
  if (humidity > 60.0) {
    kitchenHighHumidity = 1;
    Serial.println("Alert: High humidity detected in the kitchen. Please check the area.");
  } else {
    kitchenHighHumidity = 0;
  }

  // Monitor MQ2 sensor
  gasValue = analogRead(gasSensorPin); // Read analog value from sensor
  float voltage = gasValue * (3.3 / 4095.0); // Convert analog value to voltage (ESP32 ADC is 12-bit)
  float gasConcentration = (voltage - 0.1) * 1000; // Convert voltage to ppm

  Serial.print("Gas Concentration: ");
  Serial.print(gasConcentration);
  Serial.println(" ppm");

  if (gasConcentration > 1800) {
    gasAlert = 1;
    Serial.print("Alert: Gas concentration out of range: ");
    Serial.print(gasConcentration);
    Serial.println(" ppm. Please check the area.");
  } else {
    gasAlert = 0;
  }

  int FlameanalogVal = analogRead(FlameanalogInPin);  // read the value of analog pin 35
  Serial.print("Analog Value: ");
  Serial.println(FlameanalogVal);  // print to serial monitor

  int FlamedigitalVal = digitalRead(FlamedigitalInPin);  // read the value of digital pin 25
  Serial.print("Digital Value: ");
  Serial.println(FlamedigitalVal);  // print to serial monitor

  if (FlamedigitalVal == LOW || FlameanalogVal < 4000) {  // assuming LOW means flame detected
    Serial.println("Alert: Flame detected! Please check the area.");
    Serial.print("Analog Flame Sensor Value: ");
    Serial.println(FlameanalogVal);
    flameAlert = 1;
  } else {
    Serial.println("No flame detected.");
    flameAlert = 0;
  }
}

void monitorBathroom() {
  int bathroomButtonState = digitalRead(buttonPinBathroom);
  // Check if the bathroom button is pressed
  if (bathroomButtonState == LOW) {
    Serial.println("Alert: Call for help from the bathroom!");
  }

  float humidity = dhtBathroom.readHumidity();
  float temperature = dhtBathroom.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from bathroom DHT sensor!");
    return;
  }

  Serial.print("Bathroom Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Bathroom Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Check for high temperature in bathroom
  if (temperature > 30.0) {
    bathroomHighTemp = 1;
    Serial.println("Alert: High temperature detected in the bathroom. Please check the area.");
  } else {
    bathroomHighTemp = 0;
  }

  // Check for high humidity in bathroom
  if (humidity > 60.0) {
    bathroomHighHumidity = 1;
    Serial.println("Alert: High humidity detected in the bathroom. Please check the area.");
  } else {
    bathroomHighHumidity = 0;
  }

  // Monitor rain sensor
  int rainValue = analogRead(rainSensorPin); // Read the analog value from the rain sensor
  Serial.print("Wet FLOORS Sensor Value: ");
  Serial.println(rainValue);

  if (rainValue < 4095) { // Adjust threshold as needed
    rainAlert = 1;
    Serial.println("Alert: Water detected in the bathroom! Please check the area.");
  } else {
    rainAlert = 0;
  }

  // Monitor touch sensor for fall detection
  int touchState = digitalRead(touchPinBathroom); // Read the state of the touch sensor
  if (touchState == HIGH) { // Assuming HIGH means the touch sensor is triggered
    fallAlert = 1;
    Serial.println("Alert: Fall detected in the bathroom! Please check the area.");
  } else {
    fallAlert = 0;
  }
}

void monitorLivingRoom() {
  int livingRoomButtonState = digitalRead(buttonPinLivingRoom);

  if (livingRoomButtonState == LOW) {
    Serial.println("Alert: Call for help from the living room!");
  }
  // Placeholder function
  int reedSwitchState = digitalRead(reedSwitchPin); // Read the state of the reed switch

  if (reedSwitchState == LOW) { // Check if the magnetic switch is closed
    Serial.println("Magnet detected!"); // Magnet is near the switch, circuit is closed
  } else {
    Serial.println("No magnet detected."); // Magnet is away from the switch, circuit is open
  }
}

void monitorBedroom() {
  int bedroomButtonState = digitalRead(buttonPinBedroom);
  // Check if the bedroom button is pressed
  if (bedroomButtonState == LOW) {
    Serial.println("Alert: Call for help from the bedroom!");
  }
}

void updateFlags() {
  Serial.println("Updating flags:");
  Serial.print("Kitchen High Temp: ");
  Serial.println(kitchenHighTemp);
  Serial.print("Kitchen High Humidity: ");
  Serial.println(kitchenHighHumidity);
  Serial.print("Bathroom High Temp: ");
  Serial.println(bathroomHighTemp);
  Serial.print("Bathroom High Humidity: ");
  Serial.println(bathroomHighHumidity);
  Serial.print("Gas Alert: ");
  Serial.println(gasAlert);
  Serial.print("Flame Alert: ");
  Serial.println(flameAlert);
  Serial.print("Wet Floors in Barthroom Alert: ");
  Serial.println(rainAlert);
  Serial.print("Fall Alert: ");
  Serial.println(fallAlert);
}
