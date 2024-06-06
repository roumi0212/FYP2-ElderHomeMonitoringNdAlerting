#include <DHT.h>

// MQ2 sensor pin
const int gasSensorPin = 34; // GPIO pin connected to MQ2 sensor
int gasValue = 0;

// DHT11 sensor configuration
#define DHTPIN 32         // GPIO pin connected to the DHT11 sensor
#define DHTTYPE DHT11    // DHT 11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200); // Start the serial communication at a baud rate of 115200
  dht.begin();          // Initialize the DHT11 sensor
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

  // Read humidity from DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Read temperature as well for logging purposes

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  if (humidity > 60.0) {
    Serial.println("Humidity level is too high! Please check the area.");
  } else {
    Serial.println("Humidity level is normal.");
      Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  }

  delay(2000); // Wait for 2 seconds before next reading
}
