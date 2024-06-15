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

// Kitchen Limits
const float hightemp_K = 35.0;
const float HighHum_K = 70.0;
const float HighGas_K = 1800;
const float HighFlame_K = 4000;

// Bathroom Limits
const float hightemp_B = 35.0;
const float HighHum_B = 70.0;
const int HighRain_B = 4000;

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
    String command = Serial.readStringUntil('\n');
    processReceivedCommand(command);
  }

    if (Serial.available() > 0) {
    int command = Serial.parseInt();
    if (command == 1) {
      alertUser();
    }
  }

  buttonState = digitalRead(stopButtonPin);
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
  if (callForHelp == LOW) {
    Serial.println("!!!!!!!!!!!!!!!!!!!!! General Call For Help from Central Unit !!!!!!!!!!!!!!!!!!!!!");
    callForHelpFlag = true;
  } else {
    callForHelpFlag = false;
  }

  monitorKitchen();
  monitorBathroom();
  monitorLivingRoom();
  monitorBedroom();

  transmitAlerts();
  delay(2000); // Wait for 2 seconds before next reading
}

void alertUser() {
  buttonState = digitalRead(stopButtonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    alertUserFlag = false; // Deactivate alert
    delay(50); // Debounce delay
  }
  lastButtonState = buttonState; // Update the last button state

  if (alertUserFlag) {
    Serial.println("################## Alert: User initiated alert!");
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    delay(500);
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
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

  delay(100); // Short delay to avoid rapid toggling
}

void monitorKitchen() {
  int kitchenButtonState = digitalRead(buttonPinKitchen);
  if (kitchenButtonState == LOW) {
    Serial.println("Alert: Call for help from the kitchen!");
  }

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

  if (temperature > hightemp_K) {
    kitchenHighTemp = true;
    Serial.println("Alert: High temperature detected in the kitchen. Please check the area.");
  } else {
    kitchenHighTemp = false;
  }

  if (humidity > HighHum_K) {
    kitchenHighHumidity = true;
    Serial.println("Alert: High humidity detected in the kitchen. Please check the area.");
  } else {
    kitchenHighHumidity = false;
  }

  gasValue = analogRead(gasSensorPin);
  float voltage = gasValue * (3.3 / 4095.0);
  float gasConcentration = (voltage - 0.1) * 1000;

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

  int FlameanalogVal = analogRead(FlameanalogInPin);
  Serial.print("Analog Value: ");
  Serial.println(FlameanalogVal);

  int FlamedigitalVal = digitalRead(FlamedigitalInPin);
  Serial.print("Digital Value: ");
  Serial.println(FlamedigitalVal);

  if (FlamedigitalVal == LOW || FlameanalogVal < HighFlame_K) {
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

  if (temperature > hightemp_B) {
    bathroomHighTemp = true;
    Serial.println("Alert: High temperature detected in the bathroom. Please check the area.");
  } else {
    bathroomHighTemp = false;
  }

  if (humidity > HighHum_B) {
    bathroomHighHumidity = true;
    Serial.println("Alert: High humidity detected in the bathroom. Please check the area.");
  } else {
    bathroomHighHumidity = false;
  }

  int rainValue = analogRead(rainSensorPin);
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  if (rainValue < HighRain_B) {
    rainAlert = true;
    Serial.println("Alert: Water detected in the bathroom! Please check the area.");
  } else {
    rainAlert = false;
  }

  int touchState = digitalRead(touchPinBathroom);
  if (touchState == HIGH) {
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

  int reedSwitchState = digitalRead(reedSwitchPin);
  if (reedSwitchState == LOW) {
    Serial.println("Magnet detected!");
  } else {
    Serial.println("No magnet detected.");
  }
}

void monitorBedroom() {
  int bedroomButtonState = digitalRead(buttonPinBedroom);
  if (bedroomButtonState == LOW) {
    Serial.println("Alert: Call for help from the bedroom!");
  }
}

void transmitAlerts() {
  if (kitchenHighTemp) {
    Serial.println("1");
  }
  if (kitchenHighHumidity) {
    Serial.println("2");
  }
  if (bathroomHighTemp) {
    Serial.println("3");
  }
  if (bathroomHighHumidity) {
    Serial.println("4");
  }
  if (gasAlert) {
    Serial.println("5");
  }
  if (flameAlert) {
    Serial.println("6");
  }
  if (rainAlert) {
    Serial.println("7");
  }
  if (fallAlert) {
    Serial.println("8");
  }
  if (callForHelpFlag) {
    Serial.println("9");
  }
  if (alertUserFlag) {
    Serial.println("10");
  }
}

void processReceivedCommand(String command) {
  if (command == "ALERT_USER") {
    alertUserFlag = true;

  }
    if (alertUserFlag) {
    alertUser();
  } else {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }
}
