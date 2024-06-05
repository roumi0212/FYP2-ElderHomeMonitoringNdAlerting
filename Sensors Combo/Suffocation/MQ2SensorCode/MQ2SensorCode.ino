const int gasSensorPin = 34; // GPIO pin connected to MQ2 sensor
int gasValue = 0;

void setup() {
  Serial.begin(115200); // Start the serial communication at a baud rate of 115200
}

void loop() {
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

  Serial.print("Gas concentration IN NORMAL range: ");
  Serial.print(gasConcentration);
  Serial.println(" ppm. ");
  }
  delay(1000); // Wait for 1 second before next reading
}
