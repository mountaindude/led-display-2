# led-display-2
Stand-alone TM1638 based 8-digit 7-segment display, driven by ESP8266 using MQTT messages


## MQTT topics
Once the firmware is on the device, all data transfer is done using MQTT messages, using well defined MQTT topics.
Configuration is done using a set of configuration topics, while data to be shown on the LED display is sent on other topics.

### Configuration messages
