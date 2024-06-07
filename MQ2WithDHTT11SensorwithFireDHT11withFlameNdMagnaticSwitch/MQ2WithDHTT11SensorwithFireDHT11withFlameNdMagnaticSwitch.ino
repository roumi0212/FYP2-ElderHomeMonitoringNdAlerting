#include <DHT.h>

// MQ2 sensor pin
const int gasSensorPin = 34; // GPIO pin connected to MQ2 sensor
int gasValue = 0;

// DHT11 sensor configuration
#define DHTPIN_KITCHEN 32 // GPIO pin connected to the DHT11 sensor in the kitchen
#define DHTPIN_BATHROOM 33 // GPIO pin connected to the DHT11 sensor in the bathroom
#define DHTTYPE DHT11    // DHT 11

DHT dhtKitchen(DHTPIN_KITCHEN, DHTTYPE);
DHT dhtBathroom(DHTPIN_BATHROOM, DHTTYPE);

void setup() {
  Serial.begin(115200); // Start the serial communication at a baud rate of 115200
  dhtKitchen.begin();   // Initialize the DHT11 sensor in the kitchen
  dhtBathroom.begin();  // Initialize the DHT11 sensor in the bathroom
}

void loop() {
  // Read gas concentration from MQ2 sensor
  gasValue = analogRead(gasSensorPin); // Read analog value from sensor
  float voltage = gasValue * (3.3 / 4095.0); // Convert analog value to voltage (ESP32 ADC is 12-bit)
  float gasConcentration = (voltage - 0.1) * 1000; // Convert voltage to ppm

  Serial.print("Gas Concentration: ");
  Serial.print(gasConcentration);
  Serial.println(" ppm");

  if (gasConcentration > 1800) {
    Serial.print("Alert: Gas concentration out of range: ");
    Serial.print(gasConcentration);
    Serial.println(" ppm. Please check the area.");
  } else {
    Serial.print("Gas concentration IN NORMAL range: ");
    Serial.print(gasConcentration);
    Serial.println(" ppm. ");
  }

  // Read temperature and humidity from kitchen sensor
  float humidityKitchen = dhtKitchen.readHumidity();
  float temperatureKitchen = dhtKitchen.readTemperature();

  // Read temperature and humidity from bathroom sensor
  float humidityBathroom = dhtBathroom.readHumidity();
  float temperatureBathroom = dhtBathroom.readTemperature();

  // Check if readings from kitchen sensor are valid
  if (isnan(humidityKitchen) || isnan(temperatureKitchen)) {
    Serial.println("Failed to read from kitchen DHT sensor!");
  } else {
    Serial.print("Kitchen Temperature: ");
    Serial.print(temperatureKitchen);
    Serial.println(" °C");

    Serial.print("Kitchen Humidity: ");
    Serial.print(humidityKitchen);
    Serial.println(" %");

    // Check for high temperature in kitchen
    if (temperatureKitchen > 70.0) {
      Serial.println("Alert: High temperature detected in the kitchen. Please check the area.");
    }

    // Check for high humidity in kitchen
    if (humidityKitchen > 60.0) {
      Serial.println("Alert: High humidity detected in the kitchen. Please check the area.");
    }
  }

  // Check if readings from bathroom sensor are valid
  if (isnan(humidityBathroom) || isnan(temperatureBathroom)) {
    Serial.println("Failed to read from bathroom DHT sensor!");
  } else {
    Serial.print("Bathroom Temperature: ");
    Serial.print(temperatureBathroom);
    Serial.println(" °C");

    Serial.print("Bathroom Humidity: ");
    Serial.print(humidityBathroom);
    Serial.println(" %");

    // Check for high temperature in bathroom
    if (temperatureBathroom > 70.0) {
      Serial.println("Alert: High temperature detected in the bathroom. Please check the area.");
    }

    // Check for high humidity in bathroom
    if (humidityBathroom > 60.0) {
      Serial.println("Alert: High humidity detected in the bathroom. Please check the area.");
    }
  }

  delay(2000); // Wait for 2 seconds before next reading
}
