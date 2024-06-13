#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
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

enum State {INITIAL, AWAITING_NAME, AWAITING_PASSWORD, MAIN_MENU};

// Map to store the state of each user
std::map<String, State> userStates;
std::map<String, String> userNames;

// Predefined password
String predefinedPassword = "password123";

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

// Button states
int kitchenButtonState = 0;
int bathroomButtonState = 0;
int livingRoomButtonState = 0;
int bedroomButtonState = 0;
int reedSwitchState = 0; // Assuming this is the state of the reed switch

void setup() {
  Serial.begin(115200);
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
}

void loop() {
  if (Serial.available() > 0) {
    int command = Serial.parseInt();
    processReceivedCommand(command);
  }

  sendAlertMessages();
  
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  // Send alertUserFlag status back to the transmitter
  if (alertUserFlag) {
    Serial.println("ALERT_USER");
    alertUserFlag = false;  // Reset the flag after sending
  }
}

void processReceivedCommand(int command) {
  switch (command) {
    case 1:
      kitchenHighTemp = true;
      break;
    case 2:
      kitchenHighHumidity = true;
      break;
    case 3:
      bathroomHighTemp = true;
      break;
    case 4:
      bathroomHighHumidity = true;
      break;
    case 5:
      gasAlert = true;
      break;
    case 6:
      flameAlert = true;
      break;
    case 7:
      rainAlert = true;
      break;
    case 8:
      fallAlert = true;
      break;
    case 9:
      callForHelpFlag = true;
      break;
    case 10:
      alertUserFlag = true;
      break;
    default:
      Serial.println("Unknown command received");
      break;
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
  if (reedSwitchState) {
    alertMessage += "Alert: Door opened outside of normal hours!\n";
  }

  // Button Calls for Help
  if (kitchenButtonState) {
    alertMessage += "Alert: Call for help from the kitchen!\n";
  }
  if (bathroomButtonState) {
    alertMessage += "Alert: Call for help from the bathroom!\n";
  }
  if (livingRoomButtonState) {
    alertMessage += "Alert: Call for help from the living room!\n";
  }
  if (bedroomButtonState) {
    alertMessage += "Alert: Call for help from the bedroom!\n";
  }
  if (callForHelpFlag) {
    alertMessage += "Alert: Call For Help from Central Unit! \n";
  }

  if (alertMessage != "") {
    for (const auto& user : userStates) {
      bot.sendMessage(user.first, alertMessage, "");
    }
  }

  for (const auto& user : userStates) {
    displayMainMenu(user.first);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (userStates.find(chat_id) == userStates.end()) {
      userStates[chat_id] = INITIAL;
    }

    State& state = userStates[chat_id];

    if (state == INITIAL) {
      bot.sendMessage(chat_id, "Please enter your name:", "");
      state = AWAITING_NAME;
    } else if (state == AWAITING_NAME) {
      userNames[chat_id] = text;
      bot.sendMessage(chat_id, "Hello " + text + ", please enter your password:", "");
      state = AWAITING_PASSWORD;
    } else if (state == AWAITING_PASSWORD) {
      if (text == predefinedPassword) {
        bot.sendMessage(chat_id, "Welcome " + userNames[chat_id] + "! You are now authorized.", "");
        state = MAIN_MENU;
        displayMainMenu(chat_id);
      } else {
        bot.sendMessage(chat_id, "Incorrect password. Please try again:", "");
      }
    } else if (state == MAIN_MENU) {
      if (bot.messages[i].type == "callback_query") {
        String callbackData = bot.messages[i].text;
        handleCallbackQuery(callbackData, chat_id);
        bot.answerCallbackQuery(bot.messages[i].query_id);
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
  // liveUpdate += callForHelpFlag ? "Emergency call for help detected.\n" : "No emergency call for help.\n";

  bot.sendMessage(chat_id, liveUpdate, "");
  Serial.println("Live update sent.");
}
