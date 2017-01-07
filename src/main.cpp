
#include <ESP8266WiFi.h>
#include <CMMC_OTA.h>
#include <PubSubClient.h>

#include <TM1638.h>
#include <InvertedTM1638.h>
#include <ArduinoJson.h>

#include "secrets.h"

CMMC_OTA ota;


#define DEBUG 1;


// Sources of inspiration

// http://www.mjoldfield.com/atelier/2012/08/pi-tm1638.html
// https://blog.3d-logic.com/2015/01/10/using-a-tm1638-based-board-with-arduino/
// https://trandi.wordpress.com/2012/10/11/tm1638-display-driver-for-stellaris-launchpad/

// define a module on data pin 8, clock pin 9 and strobe pin 7
// define a module on data pin D3, clock pin D2 and strobe pin D4
TM1638 module(D3, D2, D4);
//InvertedTM1638 module(D3, D2, D4);

// Can be used to keep an uptime counter within the app.
unsigned long startTime;


// MQTT related variables
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Buttons pressed during last scan
byte lastButtons = 0;




void init_wifi_hardware()
{
  byte dotPosition = 0;
  module.setLED(TM1638_COLOR_RED, dotPosition);

  WiFi.disconnect(true);
  Serial.begin(115200);
  Serial.flush();
  Serial.println();
  Serial.println("Starting wifi...");
  delay(100);

  // Connect to wifi, using red LEDs on display board as progress indicator
  WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("connecting %s:%s \r\n", WIFI_SSID, WIFI_PASSPHRASE);
    delay(1000);

    module.setLED(0, dotPosition);
    dotPosition++;
    if(dotPosition > 8) {
      dotPosition = 0;
    }
    module.setLED(TM1638_COLOR_RED, dotPosition);
  }

  module.setLEDs(0xFF00);                 // Set all green LEDs to on
  Serial.print("READY!! IP address: ");
  Serial.println(WiFi.localIP());

  // Show success message on LED display, then clear it
  module.setDisplayToString("IP done");
  delay(3000);
  module.clearDisplay();
  module.setLEDs(0x00);
}



void callback(char* topic, byte* payload, unsigned int length) {
  #ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  #endif

  String strTopic = topic;
  if (strTopic == mqttTopicMsg) {
    #ifndef DEBUG
      Serial.println("Found msg...");
    #endif

    // Get payload in shape
    char payloadStr[length+1];
    strcpy(payloadStr, (const char *) payload);
    payloadStr[length] = 0;
    String input = payloadStr;

    // Memory pool for JSON object tree
    // Inside the brackets, 200 is the size of the pool in bytes,
    // If the JSON object is more complex, you need to increase that value.
    StaticJsonBuffer<200> jsonBuffer;

    // Root of the object tree
    // It's a reference to the JsonObject, the actual bytes are inside the
    // JsonBuffer with all the other nodes of the object tree.
    // Memory is freed when jsonBuffer goes out of scope.
    JsonObject& root = jsonBuffer.parseObject(input);

    // Test if parsing succeeds
    if (!root.success()) {
      Serial.println("parseObject() failed");
      module.setDisplayToError();
      return;
    }

    #ifdef DEBUG
      Serial.println("JSON parsing ok");
      module.setDisplayToString("JSON ok");
      delay(1000);
    #endif

    // Fetch values
    // Most of the time, you can rely on the implicit casts.
    // In other case, you can do root["time"].as<long>();
    String msgType = root["type"];
    String msgValue = root["value"];

    // Print values
    Serial.println(msgType);
    Serial.println(msgValue);

    // Send to LEDs after first clearing display
    module.clearDisplay();
    module.setDisplayToString(msgValue);
  }

}


void setup() {
  module.setupDisplay(true, 7);

  init_wifi_hardware();

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("led-display-2");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *) "m0rran");
  ArduinoOTA.setPassword(otaPwd);

  ota.init();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(2000);

  startTime = millis();
}

char* string2char(String command){
  if(command.length()!=0){
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void update(TM1638* module, byte* displayButtonStatus) {
  byte buttons = module->getButtons();
  unsigned long uptimeSecs = (millis() - startTime) / 1000;

  // button pressed - post notification to MQTT
  if (buttons != 0) {
    if(buttons != lastButtons) {
      Serial.println("New button(s) pressed");
      lastButtons = buttons;
      String lastButtonsString = String(lastButtons, DEC);
      client.publish(mqttTopicButtonsPressed, string2char(lastButtonsString));
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected to MQTT server.");
      // Once connected, publish an announcement...
      client.publish(mqttTopicGeneral, "LED display coming online...");

      // ... and resubscribe
      client.subscribe(mqttTopicGeneral);

      // Subscribe to message topics
      client.subscribe(mqttTopicMsg);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  ota.loop();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  update(&module, &lastButtons);
}
