#include <NewPing.h>

// Define pins and constants
const int trigPin = 27;
const int echoPin = 26;
const int pirPin = 25;
const int buzzerPin = 14;
const int rainSensorPin = 34; // Analog pin for rain sensor
const int MAX_DISTANCE = 200; // Maximum distance for ultrasonic sensor

// Initialize NewPing instance
NewPing sonar(trigPin, echoPin, MAX_DISTANCE);
int pirState = LOW;  // Initialize PIR state
bool personInBathroom = false;

void setup() {
  Serial.begin(115200);
  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  delay(50); // Wait 50ms between pings (about 20 pings/sec)
  unsigned int distance = sonar.ping_cm();
  pirState = digitalRead(pirPin);
  int rainValue = analogRead(rainSensorPin);

  // Display sensor readings
  Serial.print("Ultrasonic Distance: ");
  Serial.print(distance);
  Serial.print(" cm, PIR State: ");
  Serial.println(pirState == HIGH ? "HIGH (Movement Detected)" : "LOW (No Movement)");
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  // Check if PIR sensor detects movement (person entering)
  if (pirState == HIGH && !personInBathroom) {
    personInBathroom = true;
    Serial.println("Person entered bathroom");
  }

  // Check if ultrasonic sensor detects the person
  if (personInBathroom && distance > 0 && distance < 25) { // Adjust threshold as needed
    Serial.println("Person detected in bathroom");
  } else if (personInBathroom && (distance == 0 || distance >= 25)) {
    // If person not detected by ultrasonic sensor, double recheck for fall detection
    int fallRecheckCounter = 0;
    delay(500);
    pirState = digitalRead(pirPin); // First recheck of PIR sensor
    if (pirState == LOW) {
      fallRecheckCounter++;
    }

    delay(500); // Wait for another 0.5 seconds for the second recheck
    pirState = digitalRead(pirPin); // Second recheck of PIR sensor
    if (pirState == LOW) {
      fallRecheckCounter++;
    }

    if (fallRecheckCounter == 2) {
      Serial.println("Fall detected!");
      digitalWrite(buzzerPin, HIGH); // Trigger alert (e.g., buzzer)
      // Add code to send alert via communication module
    }
  }

  // Check if rain sensor detects wet conditions
  if (rainValue < 4095) { // Adjust threshold as needed
    Serial.println("Wet floors detected!");
  }

  // Check for exit confirmation
  if (personInBathroom && (distance == 0 || distance >= 25)) {
    int exitRecheckCounter = 0;
    delay(500); // Wait for 2 seconds before the first recheck
    pirState = digitalRead(pirPin); // First recheck of PIR sensor
    if (pirState == LOW) {
      exitRecheckCounter++;
    }

    delay(500); // Wait for another 2 seconds for the second recheck
    pirState = digitalRead(pirPin); // Second recheck of PIR sensor
    if (pirState == HIGH) {
      exitRecheckCounter++;
    }

    if (exitRecheckCounter == 2) {
      personInBathroom = false;
      digitalWrite(buzzerPin, LOW);
      Serial.println("Person exited bathroom");
    }
  }

  delay(1000); // Adjust delay as needed
}
