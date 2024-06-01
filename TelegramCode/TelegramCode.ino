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

enum State {INITIAL, AWAITING_GUEST_NAME, AWAITING_MEMBER_NAME, AWAITING_PASSWORD, MAIN_MENU};
State botState = INITIAL;
String guestName = "";
String selectedMember = "";

// List of verified users
std::vector<String> verifiedUsers;

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

    // Check if the user is already verified
    if (std::find(verifiedUsers.begin(), verifiedUsers.end(), chat_id) == verifiedUsers.end()) {
      // User is not verified, prompt for verification
      if (bot.messages[i].type == F("callback_query")) {
        handleCallbackQuery(bot.messages[i].text, chat_id);
      } else {
        if (botState == INITIAL) {
          displayMemberMenu(chat_id);
          botState = INITIAL;
        } else if (botState == AWAITING_GUEST_NAME) {
          guestName = text;
          bot.sendMessage(chat_id, "Hello " + guestName + ", please choose an action from the menu:", "");
          displayMainMenu(chat_id);
          verifiedUsers.push_back(chat_id);
          botState = MAIN_MENU;
        } else if (botState == AWAITING_MEMBER_NAME) {
          selectedMember = text;
          if (selectedMember == "Mariam" || selectedMember == "Yasser") {
            bot.sendMessage(chat_id, "Please enter your password:", "");
            botState = AWAITING_PASSWORD;
          } else {
            bot.sendMessage(chat_id, "Invalid member name. Please choose Mariam or Yasser:", "");
          }
        } else if (botState == AWAITING_PASSWORD) {
          if ((selectedMember == "Mariam" && text == "mariam123") || (selectedMember == "Yasser" && text == "yasser123")) {
            bot.sendMessage(chat_id, "Welcome " + selectedMember + "! You are now authorized.", "");
            displayMainMenu(chat_id);
            verifiedUsers.push_back(chat_id);
            botState = MAIN_MENU;
          } else {
            bot.sendMessage(chat_id, "Incorrect password. Please try again:", "");
          }
        }
      }
    } else {
      // User is verified, handle their messages
      if (bot.messages[i].type == F("callback_query")) {
        handleCallbackQuery(bot.messages[i].text, chat_id);
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

void displayFamilyMemberMenu(const String &chat_id) {
  String menu = "Please choose your name:";
  String keyboardJson = "[[{\"text\":\"Mariam\",\"callback_data\":\"Mariam\"}],"
                         "[{\"text\":\"Yasser\",\"callback_data\":\"Yasser\"}]]";
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

void handleCallbackQuery(String callbackData, const String &chat_id) {
  if (callbackData == "family_member") {
    displayFamilyMemberMenu(chat_id);
    botState = AWAITING_MEMBER_NAME;
  } else if (callbackData == "guest") {
    bot.sendMessage(chat_id, "Please enter your name:", "");
    botState = AWAITING_GUEST_NAME;
  } else if (callbackData == "Mariam") {
    selectedMember = "Mariam";
    bot.sendMessage(chat_id, "Please enter your password:", "");
    botState = AWAITING_PASSWORD;
  } else if (callbackData == "Yasser") {
    selectedMember = "Yasser";
    bot.sendMessage(chat_id, "Please enter your password:", "");
    botState = AWAITING_PASSWORD;
  } else if (callbackData == "alert_house") {
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
