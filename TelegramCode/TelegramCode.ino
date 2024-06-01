#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi network credentials
#define WIFI_SSID "Mariam's"
#define WIFI_PASSWORD "mariamshotspotyall"

// Telegram BOT Token and Chat ID
#define BOT_TOKEN "6704040514:AAEqdRFRhIXZtbBqArfKGl3xQ8WFq1K4Q0c"
#define CHAT_ID "1046014187" // Replace with your actual chat ID

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime = 0; // last time messages' scan has been done

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

  // Send an initial alert message based on the status flags
  sendAlertMessages();
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
  if (statusFalls == 1) {
    bot.sendMessage(CHAT_ID, "Alert! Risk of Falls detected.", "");
  }
  if (statusSuffocation == 1) {
    bot.sendMessage(CHAT_ID, "Alert! Risk of Suffocation (Poisoning) detected.", "");
  }
  if (statusFire == 1) {
    bot.sendMessage(CHAT_ID, "Alert! Fire Hazard detected.", "");
  }
  if (statusCognitive == 1) {
    bot.sendMessage(CHAT_ID, "Alert! Risk of Cognitive Decline detected.", "");
  }
  if (statusEmergency == 1) {
    bot.sendMessage(CHAT_ID, "Alert! Other Emergency (Manual) detected.", "");
  }

  // Display the menu after sending the alert
  displayMenu();
}

void displayMenu() {
  String menu = "Please choose an action:\n";
  String keyboardJson = "[[{\"text\":\"Alert the House\",\"callback_data\":\"alert_house\"}],"
                         "[{\"text\":\"Call Emergency Care Provider\",\"callback_data\":\"call_care\"}],"
                         "[{\"text\":\"Call Resident of House\",\"callback_data\":\"call_resident\"}],"
                         "[{\"text\":\"Request Live Update\",\"callback_data\":\"live_update\"}]]";
  bot.sendMessageWithInlineKeyboard(CHAT_ID, menu, "", keyboardJson);
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
    } else {
      if (bot.messages[i].type == "callback_query") {
        String callbackData = bot.messages[i].callback_query.data;
        handleCallbackQuery(callbackData);
        bot.answerCallbackQuery(bot.messages[i].callback_query.id); // This line is important to send a response to the callback
      } else if (bot.messages[i].text == "/start") {
        // Display the menu when the /start command is received
        displayMenu();
      } else {
        // Display the menu for any unrecognized text message
        bot.sendMessage(CHAT_ID, "Command not recognized. Please choose an action from the menu.", "");
        displayMenu();
      }
    }
  }
}

void handleCallbackQuery(String callbackData) {
  if (callbackData == "alert_house") {
    alertHouse(bot, CHAT_ID);
  } else if (callbackData == "call_care") {
    callEmergencyCareProvider(bot, CHAT_ID);
  } else if (callbackData == "call_resident") {
    callResident(bot, CHAT_ID);
  } else if (callbackData == "live_update") {
    sendLiveUpdate(bot, CHAT_ID);
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
