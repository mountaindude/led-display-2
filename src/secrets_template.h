
// Instructions: Rename or copy file to secrets.h, then edit the file as needed to match your specific environment

// Credentials for wifi network the ESP8266 should connect to
#ifndef WIFI_SSID
  #define WIFI_SSID       "SSID-name"
  #define WIFI_PASSPHRASE "MyWifiPassword"
#endif

// Pwd for OTA updates of ESP8266
const char* otaPwd = "myOTAPassW0rd";

// IP of MQTT server
const char* mqtt_server = "123.123.123.123";

// MQTT topics to be used
const char* mqttTopicGeneral = "ptarmiganlabs/led-display-2/general";
const char* mqttTopicMsg = "ptarmiganlabs/led-display-2/msg";
const char* mqttTopicViews = "ptarmiganlabs/led-display-2/view/set";
const char* mqttTopicButtonsPressed = "ptarmiganlabs/led-display-2/buttonsPressed";
const char* mqttTopicDumpViews = "ptarmiganlabs/led-display-2/dumpViews";
const char* mqttTopicDumpViewsData = "ptarmiganlabs/led-display-2/dumpViews/data";
