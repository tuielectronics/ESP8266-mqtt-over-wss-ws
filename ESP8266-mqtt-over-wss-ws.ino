/*
 * powered by Tui Electronics
 * go to GLOBAL_VAR to config your setting before flash
   created on 2019 Mar 28
*/
#include "GLOBAL_VAR.h" // global variables

#include "WIFI_CONF.CPP" // WiFi config

#include "MQTT_BYTES.CPP" // MQTT protocol text generator

#include "WSS-SOCKETS.CPP" // security client based wss socket

#include "JSON-PARSER.CPP" // MQTT JSON message parser

#include "WS-LOOP.CPP" // websocket main loop

void setup() {
  Serial.begin(115200);
  SERIAL_DEBUG("\nConnect to %s\n", wifiPassword);
  wifiInit();
  wsStatus = WS_NOT_START;
  wssConnect();
}

void loop() {
  long timeNowLoop = millis();
  wssMainLoop(timeNowLoop);
}
