# define WS_NOT_READY 0
# define WS_NOT_START 1
# define WS_GET_SENT 2
# define WS_GET_HEADER_SWITCH 3
// # define WS_GET_HEADER_DATE 3
# define WS_HANDSHAKE_FINISHED 4
# define WS_MQTT_CONNECT_SENT 5
# define WS_MQTT_CONNECTED 6

#define WS_KEYWORD_V_PONG 0x8A
#define WS_KEYWORD_V_BIN 0x82
#define WS_KEYWORD_V_MQTT_CONNECTED_BIT1 0x20
#define WS_KEYWORD_V_MQTT_CONNECTED_BIT2 0x02
#define WS_KEYWORD_V_MQTT_PONG_BIT1 0xD0
#define WS_KEYWORD_V_MQTT_PONG_BIT2 0x00
#define WS_KEYWORD_V_MQTT_SUBACK 0x90
#define WS_KEYWORD_V_MQTT_SUBFAIL 0x80
#define WS_KEYWORD_V_MQTT_UNSUBACK 0xB0
#define WS_KEYWORD_V_MQTT_RECEIVE 0x30



#define WS_BUFFER_LENGTH 512

const static uint8_t wsPingStr[] = {0x89, 0x83, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03};
const static uint8_t wsMqttPingStr[] = {0x82, 0x82, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00};
static uint8_t wsBufferRead[WS_BUFFER_LENGTH];

static byte wsStatus = WS_NOT_READY;
String wsKeyClient = "";

uint8_t *wsSendBinParse(uint8_t *p, int sizeP) {
  uint8_t wsBinPrefix[] = {0x82, 0xD1, 0x00, 0x00, 0x00, 0x00};
  uint8_t result[sizeP + sizeof(wsBinPrefix)];
  memcpy(result, wsBinPrefix, sizeof(wsBinPrefix));
  memcpy(result + sizeof(wsBinPrefix), p, sizeP);
  return result;
}

void wsSendHeartBeat() {
  sClient.write(wsPingStr, sizeof(wsPingStr));
  SERIAL_DEBUG("[WSS CLIENT] sent ping at %d\n", millis());
}
void wsSendMqttHeartBeat() {
  sClient.write(wsMqttPingStr, sizeof(wsMqttPingStr));
  SERIAL_DEBUG("[MQTT] sent MQTT ping at %d\n", millis());
}
void wsSendMqttConnect() {
  String mqttID = String("TUIELECTRONICS-") + String(millis()) + String("-") + WiFi.macAddress();
  generateStringMqttConnect(mqttID.c_str(), MQTT_ALIVE_TIME, MQTTUsername, MQTTPassword);
  uint8_t wsBinPrefix[] = {0x82, 128 + bufferMqttGeneralCommandLength, 0x00, 0x00, 0x00, 0x00};
  uint8_t result[bufferMqttGeneralCommandLength + sizeof(wsBinPrefix)];
  memcpy(&result[0], wsBinPrefix, sizeof(wsBinPrefix));
  memcpy(&result[sizeof(wsBinPrefix)], bufferMqttGeneralCommand, bufferMqttGeneralCommandLength);
  SERIAL_DEBUG("[MQTT] connect bytes -> ");
  for (int i = 0; i < sizeof(result); i++) {
    SERIAL_DEBUG("0x%02X,", result[i]);
  }
  SERIAL_DEBUG("\n");
  sClient.write(result, sizeof(result));
}
void wsSendMqttPublish(const char* messageTopic, const char* messagePayload) {
  generateStringMqttPublish(messageTopic, messagePayload);
  //http://lucumr.pocoo.org/2012/9/24/websockets-101/
  if (bufferMqttGeneralCommandLength <= 125) {
    uint8_t wsBinPrefix[] = {0x82, 128 + bufferMqttGeneralCommandLength, 0x00, 0x00, 0x00, 0x00};
    uint8_t result[bufferMqttGeneralCommandLength + sizeof(wsBinPrefix)];
    memcpy(&result[0], wsBinPrefix, sizeof(wsBinPrefix));
    memcpy(&result[sizeof(wsBinPrefix)], bufferMqttGeneralCommand, bufferMqttGeneralCommandLength);
    sClient.write(result, sizeof(result));
  }
  else {
    uint8_t wsBinPrefix[] = {0x82, 128 + 126, (uint8_t)(bufferMqttGeneralCommandLength / 256), (uint8_t)(bufferMqttGeneralCommandLength % 256), 0x00, 0x00, 0x00, 0x00};
    uint8_t result[bufferMqttGeneralCommandLength + sizeof(wsBinPrefix)];
    memcpy(&result[0], wsBinPrefix, sizeof(wsBinPrefix));
    memcpy(&result[sizeof(wsBinPrefix)], bufferMqttGeneralCommand, bufferMqttGeneralCommandLength);
    sClient.write(result, sizeof(result));
  }
  SERIAL_DEBUG("[MQTT] mqtt publish -> %s, %s <-\n", messageTopic, messagePayload);
}
void wsSendMqttSubUnsub(const char* messageTopic, bool subUnsub) {
  generateStringMqttSubscribe(messageTopic, subUnsub);
  uint8_t wsBinPrefix[] = {0x82, 128 + bufferMqttGeneralCommandLength, 0x00, 0x00, 0x00, 0x00};
  uint8_t result[bufferMqttGeneralCommandLength + sizeof(wsBinPrefix)];
  memcpy(&result[0], wsBinPrefix, sizeof(wsBinPrefix));
  memcpy(&result[sizeof(wsBinPrefix)], bufferMqttGeneralCommand, bufferMqttGeneralCommandLength);
  sClient.write(result, sizeof(result));
  SERIAL_DEBUG("[MQTT] mqtt %s to <-- %s\n", subUnsub ? "subscribe" : "unsubscribe", messageTopic);
}
