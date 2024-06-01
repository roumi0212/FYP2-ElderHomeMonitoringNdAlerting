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

enum State {INITIAL, AWAITING_GUEST_NAME, MAIN_MENU};
State botState = INITIAL;
String guestName = "";

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
  // Add your existing handling of serial inputs here
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (botState == INITIAL) {
      displayMemberMenu(chat_id);
      botState = MAIN_MENU;
    } else if (botState == AWAITING_GUEST_NAME) {
      guestName = text;
      bot.sendMessage(chat_id, "Hello " + guestName + ", please choose an action from the menu:", "");
      displayMainMenu(chat_id);
      botState = MAIN_MENU;
    } else if (botState == MAIN_MENU) {
      if (bot.messages[i].type == "callback_query") {
        String callbackData = bot.messages[i].text;
        handleCallbackQuery(callbackData);
      } else {
        displayMainMenu(chat_id);
      }
    }
  }
}

void displayMemberMenu(const String &chat_id) {
  String menu = "Are you a family member or a guest?";
  String keyboardJson = "[[{\"text\":\"Family Member\",\"callback_data\":\"family_member\"}],"
                         "[{\"text\":\"Guest\",\"callback_data\":\"guest\"}]]";
  bot.sendMessageWithInlineKeyboard(chat_id, menu, "", keyboardJson);
}

void displayMainMenu(const String &chat_id) {
  String menu = "Please choose an action:";
  String keyboardJson = "[[{\"text\":\"Alert the House\",\"callback_data\":\"alert_house\"}],"
                         "[{\"text\":\"Call Emergency Care Provider\",\"callback_data\":\"call_care\"}],"
                         "[{\"text\":\"Call Resident of House\",\"callback_data\":\"call_resident\"}],"
                         "[{\"text\":\"Request Live Update\",\"callback_data\":\"live_update\"}]]";
  bot.sendMessageWithInlineKeyboard(chat_id, menu, "", keyboardJson);
}

void handleCallbackQuery(String callbackData) {
  if (callbackData == "family_member") {
    bot.sendMessage(CHAT_ID, "Welcome family member! Please choose an action from the menu:", "");
    displayMainMenu(CHAT_ID);
  } else if (callbackData == "guest") {
    bot.sendMessage(CHAT_ID, "Please enter your name:", "");
    botState = AWAITING_GUEST_NAME;
  } else if (callbackData == "alert_house") {
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
