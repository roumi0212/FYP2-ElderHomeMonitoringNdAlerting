#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <map>
#include <ctime>

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
int statusFalls = 0;
int statusSuffocation = 0;
int statusFire = 0;
int statusCognitive = 0;
int statusEmergency = 0;

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
    String input = Serial.readStringUntil('\n');
    handleSerialInput(input);
  }

  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}

void handleSerialInput(String input) {
  input.trim();
  if (input.equalsIgnoreCase("Falls")) {
    statusFalls = 1;
  } else if (input.equalsIgnoreCase("Suffocation")) {
    statusSuffocation = 1;
  } else if (input.equalsIgnoreCase("Fire")) {
    statusFire = 1;
  } else if (input.equalsIgnoreCase("Cognitive")) {
    statusCognitive = 1;
  } else if (input.equalsIgnoreCase("Emergency")) {
    statusEmergency = 1;
  }

  // Send alert messages based on the updated status flags
  sendAlertMessages();

  // Reset status flags after sending the alert
  statusFalls = 0;
  statusSuffocation = 0;
  statusFire = 0;
  statusCognitive = 0;
  statusEmergency = 0;
}

void sendAlertMessages() {
  String alertMessage = "";
  if (statusFalls == 1) {
    alertMessage += "Alert! Risk of Falls detected.\n";
  }
  if (statusSuffocation == 1) {
    alertMessage += "Alert! Risk of Suffocation (Poisoning) detected.\n";
  }
  if (statusFire == 1) {
    alertMessage += "Alert! Fire Hazard detected.\n";
  }
  if (statusCognitive == 1) {
    alertMessage += "Alert! Risk of Cognitive Decline detected.\n";
  }
  if (statusEmergency == 1) {
    alertMessage += "Alert! Other Emergency (Manual) detected.\n";
  }

  if (alertMessage != "") {
    for (const auto& user : userStates) {
      bot.sendMessage(user.first, alertMessage, "");
    }
  }

  // Display the menu after sending the alert
  for (const auto& user : userStates) {
    displayMainMenu(user.first);
  }
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    // Initialize user state if not already present
    if (userStates.find(chat_id) == userStates.end()) {
      userStates[chat_id] = INITIAL;
      Serial.print("New user detected: ");
      Serial.print("Chat ID: ");
      Serial.print(chat_id);
      Serial.print(", Time: ");
      time_t now = time(0);
      String dt = ctime(&now);
      Serial.println(dt);
    }

    State& state = userStates[chat_id];

    if (state == INITIAL) {
      bot.sendMessage(chat_id, "Please enter your name:", "");
      state = AWAITING_NAME;
    } else if (state == AWAITING_NAME) {
      userNames[chat_id] = text;
      bot.sendMessage(chat_id, "Hello " + text + ", please enter your password:", "");
      Serial.print("User ");
      Serial.print(text);
      Serial.print(" (Chat ID: ");
      Serial.print(chat_id);
      Serial.print(") entered their name at ");
      time_t now = time(0);
      String dt = ctime(&now);
      Serial.println(dt);
      state = AWAITING_PASSWORD;
    } else if (state == AWAITING_PASSWORD) {
      if (text == predefinedPassword) {
        bot.sendMessage(chat_id, "Welcome " + userNames[chat_id] + "! You are now authorized.", "");
        displayMainMenu(chat_id);
        state = MAIN_MENU;
      } else {
        bot.sendMessage(chat_id, "Incorrect password. Please try again:", "");
      }
    } else if (state == MAIN_MENU) {
      if (bot.messages[i].type == "callback_query") {
        String callbackData = bot.messages[i].text;
        handleCallbackQuery(callbackData, chat_id);
        bot.answerCallbackQuery(bot.messages[i].query_id);
      } else {
        displayMainMenu(chat_id);
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
  bot.sendMessage(chat_id, "Providing live update...", "");
  Serial.println("Providing live update...");
  // Add your live update functionality here
}
