#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// WiFi network credentials
#define WIFI_SSID "Mariam's"
#define WIFI_PASSWORD "mariamshotspotyall"

// Telegram BOT Token and Chat ID
#define BOT_TOKEN "6704040514:AAEqdRFRhIXZtbBqArfKGl3xQ8WFq1K4Q0c"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime = 0; // last time messages' scan has been done

enum State {INITIAL, AWAITING_NAME, AWAITING_PASSWORD, MAIN_MENU};
State botState = INITIAL;
String userName = "";

// List of active chat IDs
std::vector<String> activeChatIDs;

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
    for (const String &chat_id : activeChatIDs) {
      bot.sendMessage(chat_id, alertMessage, "");
    }
  }

  // Display the menu after sending the alert
  for (const String &chat_id : activeChatIDs) {
    displayMainMenu(chat_id);
  }
}

void handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        // Add chat ID to active list if not already present
        if (std::find(activeChatIDs.begin(), activeChatIDs.end(), chat_id) == activeChatIDs.end()) {
            activeChatIDs.push_back(chat_id);
        }

        if (botState == INITIAL) {
            bot.sendMessage(chat_id, "Please enter your name:", "");
            botState = AWAITING_NAME;
        } else if (botState == AWAITING_NAME) {
            userName = text;
            bot.sendMessage(chat_id, "Hello " + userName + ", pleas
