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
const int touchPinBathroom = 19; // GPIO pin connected to the touch sensor for fall detection in bathroom
const int alertButtonPin = 5; // GPIO pin connected to the user alert button
const int stopButtonPin = 4; // GPIO pin connected to the stop alert button

// Rain sensor pin
const int rainSensorPin = 26; // Analog pin for rain sensor

// Buzzer and RGB LED pins
const int buzzerPin = 0;
const int redPin = 21;
const int greenPin = 22;
const int bluePin = 23;

// Status flags
bool kitchenHighTemp = false;
bool kitchenHighHumidity = false;
bool bathroomHighTemp = false;
bool bathroomHighHumidity = false;
bool gasAlert = false;
bool flameAlert = false;
bool rainAlert = false; // Rain sensor alert
bool fallAlert = false; // Fall detection alert
bool callForHelpFlag = false;
bool alertUserFlag = false;

bool buttonState = false;   // State of the button
bool lastButtonState = false; // Previous state of the button

// // Kitchen Limits
const float hightemp_K = 35.0;
const float HighHum_K = 70.0;
const float HighGas_K = 1800;
const float HighFlame_K = 4000;

// // Bathroom Limits
const float hightemp_B = 35.0;
const float HighHum_B = 70.0;
const int HighRain_B = 3000;

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
  pinMode(stopButtonPin, INPUT_PULLUP); // Set the stop button pin as input with internal pull-up resistor
  pinMode(buzzerPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(alertButtonPin, INPUT_PULLUP); // Set the CallforHelp button pin as input with internal pull-up resistor

  // Set initial state of RGB LED pins to HIGH (turn off LED)

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  Serial.println("System Initialized");
}

void loop() {


  if (Serial.available() > 0) {
    int command = Serial.parseInt();
    if (command == 1) {
      alertUserFlag = true;
    }
  }

  buttonState = digitalRead(stopButtonPin);
  // Check if the button has been pressed (transition from HIGH to LOW)
  if (buttonState == LOW && lastButtonState == HIGH) {
    alertUserFlag = false; // Deactivate alert
    delay(50); // Debounce delay
  }
  lastButtonState = buttonState; // Update the last button state

  if (alertUserFlag) {
    alertUser();
  } else {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

  int callForHelp = digitalRead(alertButtonPin);
  if (callForHelp == LOW) { // Assuming LOW means the button is pressed
    Serial.println("!!!!!!!!!!!!!!!!!!!!! General Call For Help from Central Unit !!!!!!!!!!!!!!!!!!!!!");
    callForHelpFlag = true;
  } else {
    callForHelpFlag = false;
  }



  monitorKitchen();
  monitorBathroom();
  monitorLivingRoom();
  monitorBedroom();
  delay(2000); // Wait for 2 seconds before next reading
}

void alertUser() {
  // Read the state of the pushbutton
  buttonState = digitalRead(stopButtonPin);

  // Check if the button has been pressed (transition from HIGH to LOW)
  if (buttonState == LOW && lastButtonState == HIGH) {
    alertUserFlag = false; // Deactivate alert
    delay(50); // Debounce delay
  }

  lastButtonState = buttonState; // Update the last button state

  if (alertUserFlag) {
    Serial.println("################## Alert: User initiated alert!");
  // Turn on the buzzer and RGB LED
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(500);

  // Turn off the buzzer and RGB LED
  digitalWrite(buzzerPin, LOW);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
  delay(500);

  digitalWrite(buzzerPin, HIGH);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
  delay(500);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
  } else {
    // Turn off the buzzer and RGB LED
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

  delay(100); // Short delay to avoid rapid toggling
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
  if (temperature > hightemp_K) {
    kitchenHighTemp = true;
    Serial.println("Alert: High temperature detected in the kitchen. Please check the area.");
  } else {
    kitchenHighTemp = false;
  }

  // Check for high humidity in kitchen
  if (humidity > HighHum_K) {
    kitchenHighHumidity = true;
    Serial.println("Alert: High humidity detected in the kitchen. Please check the area.");
  } else {
    kitchenHighHumidity = false;
  }

  // Monitor MQ2 sensor
  gasValue = analogRead(gasSensorPin); // Read analog value from sensor
  float voltage = gasValue * (3.3 / 4095.0); // Convert analog value to voltage (ESP32 ADC is 12-bit)
  float gasConcentration = (voltage - 0.1) * 1000; // Convert voltage to ppm

  Serial.print("Gas Concentration: ");
  Serial.print(gasConcentration);
  Serial.println(" ppm");

  if (gasConcentration > HighGas_K) {
    gasAlert = true;
    Serial.print("Alert: Gas concentration out of range: ");
    Serial.print(gasConcentration);
    Serial.println(" ppm. Please check the area.");
  } else {
    gasAlert = false;
  }

  int FlameanalogVal = analogRead(FlameanalogInPin);  // read the value of analog pin 35
  Serial.print("Analog Value: ");
  Serial.println(FlameanalogVal);  // print to serial monitor

  int FlamedigitalVal = digitalRead(FlamedigitalInPin);  // read the value of digital pin 25
  Serial.print("Digital Value: ");
  Serial.println(FlamedigitalVal);  // print to serial monitor

  if (FlamedigitalVal == LOW || FlameanalogVal < HighFlame_K) {  // assuming LOW means flame detected
    Serial.println("Alert: Flame detected! Please check the area.");
    Serial.print("Analog Flame Sensor Value: ");
    Serial.println(FlameanalogVal);
    flameAlert = true;
  } else {
    Serial.println("No flame detected.");
    flameAlert = false;
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
  if (temperature > hightemp_B) {
    bathroomHighTemp = true;
    Serial.println("Alert: High temperature detected in the bathroom. Please check the area.");
  } else {
    bathroomHighTemp = false;
  }

  // Check for high humidity in bathroom
  if (humidity > HighHum_B) {
    bathroomHighHumidity = true;
    Serial.println("Alert: High humidity detected in the bathroom. Please check the area.");
  } else {
    bathroomHighHumidity = false;
  }

  // Monitor rain sensor
  int rainValue = analogRead(rainSensorPin); // Read the analog value from the rain sensor
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  if (rainValue < HighRain_B) { // Adjust threshold as needed
    rainAlert = true;
    Serial.println("Alert: Water detected in the bathroom! Please check the area.");
  } else {
    rainAlert = false;
  }

  // Monitor touch sensor for fall detection
  int touchState = digitalRead(touchPinBathroom); // Read the state of the touch sensor
  if (touchState == HIGH) { // Assuming HIGH means the touch sensor is triggered
    fallAlert = true;
    Serial.println("Alert: Fall detected in the bathroom! Please check the area.");
  } else {
    fallAlert = false;
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
  Serial.println("********************Updating flags:");
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
  Serial.print("Rain Alert: ");
  Serial.println(rainAlert);
  Serial.print("Fall Alert: ");
  Serial.println(fallAlert);
}


