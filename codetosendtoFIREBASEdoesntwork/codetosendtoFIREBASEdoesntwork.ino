#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <map>

// WiFi network credentials
#define WIFI_SSID "Mariam's"
#define WIFI_PASSWORD "mariamshotspotyall"

// Telegram BOT Token
#define BOT_TOKEN "6704040514:AAEqdRFRhIXZtbBqArfKGl3xQ8WFq1K4Q0c"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime = 0; // last time messages' scan has been done

enum State {INITIAL, MAIN_MENU};

// // Map to store the state of each user
std::map<String, State> userStates;
// std::map<String, String> userNames;

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
const int rainSensorPin = 36; // Analog pin for rain sensor

// Buzzer and RGB LED pins
const int buzzerPin = 0;
const int redPin = 21;
const int greenPin = 22;
const int bluePin = 23;

// Reed switch pin
const int reedSwitchPin = 27; // GPIO pin connected to the magnetic switch

// Kitchen Limits
const float hightemp_K = 35.0;
const float HighHum_K = 75.0;
const float HighGas_K = 1800;
const float HighFlame_K = 3500;

// Bathroom Limits
const float hightemp_B = 35.0;
const float HighHum_B = 75.0;
const int HighRain_B = 4000;

bool buttonState = false;   // State of the button
bool lastButtonState = false; // Previous state of the button

// Button states
int kitchenButtonState = 1; // Initialize to HIGH (not pressed)
int bathroomButtonState = 1; // Initialize to HIGH (not pressed)
int livingRoomButtonState = 1; // Initialize to HIGH (not pressed)
int bedroomButtonState = 1; // Initialize to HIGH (not pressed)
int reedSwitchState = 1; // Initialize to HIGH (not activated)

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 17, 16); // TX to GPIO 17, RX to GPIO 16
  
  Serial.print("Connecting to WiFi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

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
  int RValue = analogRead(rainSensorPin);
  Serial.print("///////////////////////////////////////////////////////////////////Rain Sensor Raw Value: ");
  Serial.println(RValue);

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

  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  sendAlertMessages();
  sendSensorData();
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
    digitalWrite(bluePin, LOW);
    delay(500);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  } else {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

  delay(50); // Short delay to avoid rapid toggling
}

void monitorKitchen() {
  kitchenButtonState = digitalRead(buttonPinKitchen);
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
  bathroomButtonState = digitalRead(buttonPinBathroom);
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

  delay(100); // Allow sensor to stabilize
  int rainValue = analogRead(rainSensorPin);
  Serial.print("Rain Sensor Raw Value: ");
  Serial.println(rainValue);

  // Ensure the threshold value is appropriate for your sensor
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
  livingRoomButtonState = digitalRead(buttonPinLivingRoom);
  if (livingRoomButtonState == LOW) {
    Serial.println("Alert: Call for help from the living room!");
  }

  reedSwitchState = digitalRead(reedSwitchPin);
  if (reedSwitchState == LOW) {
    Serial.println("Magnet detected!");
  } else {
    Serial.println("No magnet detected.");
  }
}

void monitorBedroom() {
  bedroomButtonState = digitalRead(buttonPinBedroom);
  if (bedroomButtonState == LOW) {
    Serial.println("Alert: Call for help from the bedroom!");
  }
}

void sendAlertMessages() {
  String alertMessage = "";

  // Kitchen Alerts
  if (kitchenHighTemp) {
    alertMessage += "Alert: High temperature detected in the kitchen. Please check the area.\n";
  }
  if (kitchenHighHumidity) {
    alertMessage += "Alert: High humidity detected in the kitchen. Please check the area.\n";
  }
  if (gasAlert) {
    alertMessage += "Alert: Gas concentration out of range.\n";
  }
  if (flameAlert) {
    alertMessage += "Alert: Flame detected! Please check the area.\n";
  }

  // Bathroom Alerts
  if (bathroomHighTemp) {
    alertMessage += "Alert: High temperature detected in the bathroom. Please check the area.\n";
  }
  if (bathroomHighHumidity) {
    alertMessage += "Alert: High humidity detected in the bathroom. Please check the area.\n";
  }
  if (rainAlert) {
    alertMessage += "Alert: Water detected in the bathroom! Please check the area.\n";
  }
  if (fallAlert) {
    alertMessage += "Alert: Fall detected in the bathroom! Please check the area.\n";
  }

  // Living Room Alerts
  if (reedSwitchState == HIGH) {
    alertMessage += "Alert: Door opened outside of normal hours!\n";
  }

  // Button Calls for Help
  if (kitchenButtonState == LOW) {
    alertMessage += "Alert: Call for help from the kitchen!\n";
  }
  if (bathroomButtonState == LOW) {
    alertMessage += "Alert: Call for help from the bathroom!\n";
  }
  if (livingRoomButtonState == LOW) {
    alertMessage += "Alert: Call for help from the living room!\n";
  }
  if (bedroomButtonState == LOW) {
    alertMessage += "Alert: Call for help from the bedroom!\n";
  }
  if (callForHelpFlag) {
    alertMessage += "Alert: Call For Help from Central Unit! \n";
  }

  if (alertMessage != "") {
    for (const auto& user : userStates) {
      bot.sendMessage(user.first, alertMessage, "");
      displayMainMenu(user.first);
    }
  }
}


void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id); // Declare chat_id here
    String text = bot.messages[i].text;

    if (userStates.find(chat_id) == userStates.end()) {
      userStates[chat_id] = INITIAL;
    }

    State& state = userStates[chat_id];

    if (state == INITIAL) {
      bot.sendMessage(chat_id, "Welcome to SeniorGuardianBot", "");
      state = MAIN_MENU;
      displayMainMenu(chat_id);
            bot.sendMessage(chat_id, "if you need anything else later, type /start to display the menu", "");

    } else if (state == MAIN_MENU) {
      if (bot.messages[i].type == "callback_query") {
        String callbackData = bot.messages[i].text;
        handleCallbackQuery(callbackData, chat_id);
        bot.answerCallbackQuery(bot.messages[i].query_id);
      } else if (text == "/start") {
        displayMainMenu(chat_id); // Redisplay the menu only if the user explicitly sends "/start"
      } else {
        // Optionally handle other text messages in MAIN_MENU state
      }
    }
  }
}


void displayMainMenu(const String &chat_id) {
  String menu = "Please choose an action:";
  String keyboardJson = "[[{\"text\":\"Alert the House\",\"callback_data\":\"alert_house\"}],"
                         "[{\"text\":\"Call Emergency Care Provider\",\"callback_data\":\"call_care\"}],"
                         "[{\"text\":\"Call Resident of House\",\"callback_data\":\"call_resident\"}],"
                         "[{\"text\":\"Request Live Update\",\"callback_data\":\"live_update\"}]]";
  bot.sendMessageWithInlineKeyboard(chat_id, menu, "", keyboardJson);
}

void handleCallbackQuery(String callbackData, const String &chat_id) {
  if (callbackData == "alert_house") {
    alertHouse(bot, chat_id);
  } else if (callbackData == "call_care") {
    callEmergencyCareProvider(bot, chat_id);
  } else if (callbackData == "call_resident") {
    callResident(bot, chat_id);
  } else if (callbackData == "live_update") {
    sendLiveUpdate(bot, chat_id);
  }
}

void alertHouse(UniversalTelegramBot &bot, const String &chat_id) {
  bot.sendMessage(chat_id, "Alerting the house...", "");
  Serial.println("Alerting the house...");
  alertUserFlag = true;
}

void callEmergencyCareProvider(UniversalTelegramBot &bot, const String &chat_id) {
  String phoneNumber = "+1234567890"; // Replace with the actual phone number
  String message = "Calling Emergency Care Provider...\n";
  message += "Phone number: " + phoneNumber + "\n";
  message += "[Click here to call](tel:" + phoneNumber + ")";
  bot.sendMessage(chat_id, message, "Markdown");
  Serial.println("Calling Emergency Care Provider...");
}

void callResident(UniversalTelegramBot &bot, const String &chat_id) {
  String phoneNumber = "+0987654321"; // Replace with the actual phone number
  String message = "Calling Resident of House...\n";
  message += "Phone number: " + phoneNumber + "\n";
  message += "[Click here to call](tel:" + phoneNumber + ")";
  bot.sendMessage(chat_id, message, "Markdown");
  Serial.println("Calling Resident of House...");
}

void sendLiveUpdate(UniversalTelegramBot &bot, const String &chat_id) {
  String liveUpdate = "Live Update:\n";

  liveUpdate += kitchenHighTemp ? "High temperature in the kitchen.\n" : "Temperature in the kitchen is normal.\n";
  liveUpdate += kitchenHighHumidity ? "High humidity in the kitchen.\n" : "Humidity in the kitchen is normal.\n";
  liveUpdate += bathroomHighTemp ? "High temperature in the bathroom.\n" : "Temperature in the bathroom is normal.\n";
  liveUpdate += bathroomHighHumidity ? "High humidity in the bathroom.\n" : "Humidity in the bathroom is normal.\n";
  liveUpdate += gasAlert ? "Gas concentration out of range.\n" : "Gas concentration is normal.\n";
  liveUpdate += flameAlert ? "Flame detected.\n" : "No flame detected.\n";
  liveUpdate += rainAlert ? "Water detected in the bathroom.\n" : "No water detected in the bathroom.\n";
  liveUpdate += fallAlert ? "Fall detected in the bathroom.\n" : "No fall detected in the bathroom.\n";

  bot.sendMessage(chat_id, liveUpdate, "");
  Serial.println("Live update sent.");
}

void sendSensorData() {
  float kitchenTemperature = dhtKitchen.readTemperature();
  float kitchenHumidity = dhtKitchen.readHumidity();
  float bathroomTemperature = dhtBathroom.readTemperature();
  float bathroomHumidity = dhtBathroom.readHumidity();
  int gasValue = analogRead(gasSensorPin);
  int flameAnalogVal = analogRead(FlameanalogInPin);
  int flameDigitalVal = digitalRead(FlamedigitalInPin);
  int touchBathroomState = digitalRead(touchPinBathroom);
  int rainValue = analogRead(rainSensorPin);

  String data = String(kitchenTemperature) + "," + String(kitchenHumidity) + "," +
                String(bathroomTemperature) + "," + String(bathroomHumidity) + "," +
                String(gasValue) + "," + String(flameAnalogVal) + "," +
                String(flameDigitalVal) + "," + String(touchBathroomState) + "," +
                String(rainValue) + "," + String(kitchenHighTemp) + "," +
                String(kitchenHighHumidity) + "," + String(bathroomHighTemp) + "," +
                String(bathroomHighHumidity) + "," + String(gasAlert) + "," +
                String(flameAlert) + "," + String(rainAlert) + "," +
                String(fallAlert) + "," + String(callForHelpFlag) + "," +
                String(alertUserFlag) + "\n";

  Serial1.print(data);
}
