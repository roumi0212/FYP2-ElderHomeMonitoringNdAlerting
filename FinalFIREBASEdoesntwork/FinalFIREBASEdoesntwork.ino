#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <ArduinoJson.h>

// Define Firebase ESP Client Library components
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// WiFi network credentials
#define WIFI_SSID "Mariam's"
#define WIFI_PASSWORD "mariamshotspotyall"

// Firebase API Key, Project ID, and user credentials
#define API_KEY "AIzaSyBVJQlmdgj4MM-kYPdMwTIhP16jmiSXhgo"
#define FIREBASE_PROJECT_ID "rdtrialhopeitworks"
#define USER_EMAIL "roumi0212@gmail.com"
#define USER_PASSWORD "qwerty67"

// Serial communication pins
#define RX_PIN 16
#define TX_PIN 17

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Print Firebase client version
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // Assign the API key
  config.api_key = API_KEY;

  // Assign the user sign-in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the callback function for the long-running token generation task
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Begin Firebase with configuration and authentication
  Firebase.begin(&config, &auth);

  // Reconnect to Wi-Fi if necessary
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Check for incoming data from Serial1
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    Serial.println("Received data: " + data); // Print received data for debugging
    sendToFirebase(data);
  }
}

void sendToFirebase(const String &data) {
  String documentPath = "EspData/SensorData";
  FirebaseJson content;

  int idx = 0;
  float kitchenTemperature = data.substring(idx, idx = data.indexOf(',', idx)).toFloat();
  float kitchenHumidity = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toFloat();
  float bathroomTemperature = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toFloat();
  float bathroomHumidity = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toFloat();
  int gasValue = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  int flameAnalogVal = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  int flameDigitalVal = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  int touchBathroomState = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  int rainValue = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool kitchenHighTemp = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool kitchenHighHumidity = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool bathroomHighTemp = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool bathroomHighHumidity = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool gasAlert = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool flameAlert = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool rainAlert = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool fallAlert = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool callForHelpFlag = data.substring(idx + 1, idx = data.indexOf(',', idx + 1)).toInt();
  bool alertUserFlag = data.substring(idx + 1).toInt();

  // Print all values for debugging
  Serial.println("kitchenTemperature: " + String(kitchenTemperature));
  Serial.println("kitchenHumidity: " + String(kitchenHumidity));
  Serial.println("bathroomTemperature: " + String(bathroomTemperature));
  Serial.println("bathroomHumidity: " + String(bathroomHumidity));
  Serial.println("gasValue: " + String(gasValue));
  Serial.println("flameAnalogVal: " + String(flameAnalogVal));
  Serial.println("flameDigitalVal: " + String(flameDigitalVal));
  Serial.println("touchBathroomState: " + String(touchBathroomState));
  Serial.println("rainValue: " + String(rainValue));
  Serial.println("kitchenHighTemp: " + String(kitchenHighTemp));
  Serial.println("kitchenHighHumidity: " + String(kitchenHighHumidity));
  Serial.println("bathroomHighTemp: " + String(bathroomHighTemp));
  Serial.println("bathroomHighHumidity: " + String(bathroomHighHumidity));
  Serial.println("gasAlert: " + String(gasAlert));
  Serial.println("flameAlert: " + String(flameAlert));
  Serial.println("rainAlert: " + String(rainAlert));
  Serial.println("fallAlert: " + String(fallAlert));
  Serial.println("callForHelpFlag: " + String(callForHelpFlag));
  Serial.println("alertUserFlag: " + String(alertUserFlag));

  content.set("fields/KitchenTemperature/doubleValue", kitchenTemperature);
  content.set("fields/KitchenHumidity/doubleValue", kitchenHumidity);
  content.set("fields/BathroomTemperature/doubleValue", bathroomTemperature);
  content.set("fields/BathroomHumidity/doubleValue", bathroomHumidity);
  content.set("fields/GasConcentration/integerValue", gasValue);
  content.set("fields/FlameAnalogValue/integerValue", flameAnalogVal);
  content.set("fields/FlameDigitalValue/integerValue", flameDigitalVal);
  content.set("fields/TouchBathroomState/booleanValue", touchBathroomState);
  content.set("fields/RainSensorValue/integerValue", rainValue);
  content.set("fields/KitchenHighTemp/booleanValue", kitchenHighTemp);
  content.set("fields/KitchenHighHumidity/booleanValue", kitchenHighHumidity);
  content.set("fields/BathroomHighTemp/booleanValue", bathroomHighTemp);
  content.set("fields/BathroomHighHumidity/booleanValue", bathroomHighHumidity);
  content.set("fields/GasAlert/booleanValue", gasAlert);
  content.set("fields/FlameAlert/booleanValue", flameAlert);
  content.set("fields/RainAlert/booleanValue", rainAlert);
  content.set("fields/FallAlert/booleanValue", fallAlert);
  content.set("fields/CallForHelpFlag/booleanValue", callForHelpFlag);
  content.set("fields/AlertUserFlag/booleanValue", alertUserFlag);

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, content.raw(), "")) {
    Serial.println("Data sent to Firebase");
  } else {
    Serial.println(fbdo.errorReason());
  }
}
