
#include <ESP8266WiFi.h>
#include <CMMC_OTA.h>
#include <PubSubClient.h>

#include <TM1638.h>
#include <InvertedTM1638.h>
#include <ArduinoJson.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

#include "secrets.h"

#include <main.h>
#include <view.h>
#include "debug.h"

CMMC_OTA ota;


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

// Hartbeat LED to show that main loop is running
boolean heartBeatLedStatus = false;


#define BUZZER_PIN D1
#define BUZZER_ON 0
#define BUZZER_OFF 1
#define ENCODER_PIN1 D5
#define ENCODER_PIN2 D6
#define ENCODER_SWITCH_PIN D0

// Rotary encoder. Avoid using pins with LEDs attached
Encoder myEnc(ENCODER_PIN1, ENCODER_PIN2);
long encPosition = -999;


VIEWS views;
byte currentViewId = 0;



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


  //
  // Serial.println(topic);


  if (strTopic == mqttTopicDumpViewsToSerial) {
    #ifdef DEBUG
      Serial.println("Dumping views to serial port...");
      Serial.println("# of views: " + views.getViewCount());
    #endif


    for (int i=0; views.getViewCount(); i++ ) {
      views.dumpToSerial();
    }
  }


  if (strTopic == mqttTopicViews) {
    #ifdef DEBUG
      Serial.println("Got view data over MQTT...");
    #endif

    // Expected payload format is JSON:
    // {id:"<id>", name:"<name>", value:"123"}

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
      module.setDisplayToString("Error 1");
      // module.setDisplayToError();
      return;
    }

    #ifdef DEBUG
      Serial.println("JSON parsing ok");

      // Blink 2nd LED green to indicate that JSON was successfully parsed
      module.setLED(TM1638_COLOR_GREEN, 5);
      // Optionally also show on display
      // module.setDisplayToString("JSON ok");

      // Wait for a bit so we have a chance to see the blinking led
      delay(500);

      // Clear LED
      module.setLED(0, 5);
      // module.clearDisplay();
    #endif

    // Fetch values
    // Most of the time, you can rely on the implicit casts.
    // In other case, you can do root["time"].as<long>();
    // byte viewId = root["id"];
    // String viewName = root["name"];
    // String viewValue = root["value"];

    // Update view array
    views.setView((byte) root["id"], root["name"], root["value"]);
  }


  if (strTopic == mqttTopicMsg) {
    #ifdef DEBUG
      Serial.println("Found msg...");
    #endif

    // Expected payload format is JSON:
    // {type:"msg value:"123"}
    // The type field is for future use - right now not used at all.

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
      module.setDisplayToString("Error 2");
      // module.setDisplayToError();
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

  // Set up encoder switch
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, BUZZER_ON);
  delay(100);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  // analogWrite(BUZZER_PIN, 254);

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

  // digitalWrite(BUZZER_PIN, BUZZER_OFF);
  // analogWrite(BUZZER_PIN, 255);

  digitalWrite(BUZZER_PIN, BUZZER_ON);
  delay(150);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  delay(50);
  digitalWrite(BUZZER_PIN, BUZZER_ON);
  delay(50);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
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
    module.setLED(TM1638_COLOR_RED, 6);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected to MQTT server.");
      // Once connected, publish an announcement...
      client.publish(mqttTopicGeneral, "LED display coming online...");

      // Subscribe to message topics
      client.subscribe(mqttTopicViews);
      client.subscribe(mqttTopicMsg);
      client.subscribe(mqttTopicDumpViewsToSerial);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  module.setLED(TM1638_COLOR_GREEN, 6);
}

static uint32_t lastCountTime = 0;

void loop() {
  ota.loop();

  long newPos = myEnc.read();
  if (newPos != encPosition) {
    encPosition = newPos;
    Serial.println(encPosition);
    String pos(encPosition);

    module.clearDisplay();
    module.setDisplayToString(pos);
  }

  // Show status encoder switch
  byte encSwitch = digitalRead(ENCODER_SWITCH_PIN);
  if(encSwitch == LOW) {
    module.setLED(TM1638_COLOR_GREEN, 0);
    encPosition = -999;
  } else
  if(encSwitch == HIGH) {
    module.setLED(TM1638_COLOR_RED, 0);
  }



  if ((millis() - lastCountTime) > 1000) {
      lastCountTime = millis();

      heartBeatLedStatus = !heartBeatLedStatus;

      if (heartBeatLedStatus==true) {
        module.setLED(TM1638_COLOR_GREEN, 7);
      } else {
        module.setLED(0, 7);
      }
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  update(&module, &lastButtons);
}
