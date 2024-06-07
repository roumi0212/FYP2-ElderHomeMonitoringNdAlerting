#include <DHT.h>

// DHT11 sensor configuration
#define DHTPIN_KITCHEN 32  // GPIO pin connected to the DHT11 sensor in the kitchen
#define DHTPIN_BATHROOM 33 // GPIO pin connected to the DHT11 sensor in the bathroom
#define DHTTYPE DHT11     // DHT 11

DHT dhtKitchen(DHTPIN_KITCHEN, DHTTYPE);
DHT dhtBathroom(DHTPIN_BATHROOM, DHTTYPE);

// MQ2 sensor pin
const int gasSensorPin = 34; // GPIO pin connected to MQ2 sensor
int gasValue = 0;
const int FlameanalogInPin = 35;  // A0 attach to pin 35
const int FlamedigitalInPin = 25; // D0 attach to pin 32


// Status flags
int kitchenHighTemp = 0;
int kitchenHighHumidity = 0;
int bathroomHighTemp = 0;
int bathroomHighHumidity = 0;
int gasAlert = 0;
int flameAlert = 0;

// int iterationCount = 0;

void setup() {
  Serial.begin(115200); // Start the serial communication at a baud rate of 115200
  dhtKitchen.begin();   // Initialize the DHT11 sensor in the kitchen
  dhtBathroom.begin();  // Initialize the DHT11 sensor in the bathroom
  pinMode(FlamedigitalInPin, INPUT);

}

void loop() {
  monitorKitchen();
  monitorBathroom();
  monitorLivingRoom();
  monitorBedroom();

  // iterationCount++;
  // if (iterationCount >= 5) {
  //   updateFlags();
  //   iterationCount = 0;
  // }

  delay(2000); // Wait for 2 seconds before next reading
}

void monitorKitchen() {
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
    Serial.println("Alert ********************************: High temperature detected in the kitchen. Please check the area.");
  } else {
    kitchenHighTemp = 0;
  }

  // Check for high humidity in kitchen
  if (humidity > 60.0) {
    kitchenHighHumidity = 1;
    Serial.println("*************************************Alert: High humidity detected in the kitchen. Please check the area.");
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
    Serial.println(" ppm. Please check the area.!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  } else {
    gasAlert = 0;
  }

  // // Monitor flame sensor
  // flameValueD = digitalRead(flameSensorPinD); // Read digital value from sensor
  // flameValueA = analogRead(flameSensorPinA); // Read digital value from sensor

  // if (flameValueD == LOW) { // Assuming LOW means flame detected
  //   flameAlert = 1;
  //   Serial.println("Alert: Flame detected in the kitchen! Please check the area.");
  //       Serial.println("Alert: Flame Value is:");
  //               Serial.println(flameValueA);

  // } else {
  //   flameAlert = 0;
  //       Serial.println("Flame Value is:");
  //               Serial.println(flameValueA);
  // }

    int FlameanalogVal = analogRead(FlameanalogInPin);  // read the value of analog pin 35
  Serial.print("Analog Value: ");
  Serial.println(FlameanalogVal);  // print to serial monitor

  int FlamedigitalVal = digitalRead(FlamedigitalInPin);  // read the value of digital pin 32
  Serial.print("Digital Value: ");
  Serial.println(FlamedigitalVal);  // print to serial monitor

  if (FlamedigitalVal == LOW || FlameanalogVal<4000) {  // assuming LOW means flame detected
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
    Serial.println("##########################3Alert: High temperature detected in the bathroom. Please check the area.");
  } else {
    bathroomHighTemp = 0;
  }

  // Check for high humidity in bathroom
  if (humidity > 60.0) {
    bathroomHighHumidity = 1;
    Serial.println("##########################3 Alert: High humidity detected in the bathroom. Please check the area.");
  } else {
    bathroomHighHumidity = 0;
  }
}

void monitorLivingRoom() {
  // Placeholder function
  Serial.println("Living Room monitoring function to be implemented manually.");
}

void monitorBedroom() {
  // Placeholder function
  Serial.println("Bedroom monitoring function to be implemented manually.");
}

void updateFlags() {
  Serial.println("//////////////////////////////////Updating flags:");
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
}
