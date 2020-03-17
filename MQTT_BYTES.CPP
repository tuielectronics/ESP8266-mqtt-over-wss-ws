/*
   topic length is restriced to size < 127
*/
#define MQTT_ALIVE_TIME 30
#define BUFFER_MQTT_LENGTH 384


static uint8_t bufferMqttGeneralCommand[BUFFER_MQTT_LENGTH];
static uint16_t bufferMqttGeneralCommandLength = 0;
static uint16_t idMqttPacket = 1;

void generateStringMqttConnect(const char* mqttClientID, uint8_t mqttAliveTime, const char* mqttUsername, const char* mqttPassword) {
  bufferMqttGeneralCommandLength = strlen(mqttClientID) + strlen(mqttUsername) + strlen(mqttPassword) + 18;
  memset(bufferMqttGeneralCommand, 0, BUFFER_MQTT_LENGTH);

  bufferMqttGeneralCommand[0] = 16;
  bufferMqttGeneralCommand[1] = bufferMqttGeneralCommandLength - 2;
  bufferMqttGeneralCommand[2] = 0;
  bufferMqttGeneralCommand[3] = 4;
  bufferMqttGeneralCommand[4] = 77;
  bufferMqttGeneralCommand[5] = 81;
  bufferMqttGeneralCommand[6] = 84;
  bufferMqttGeneralCommand[7] = 84;
  bufferMqttGeneralCommand[8] = 4;
  bufferMqttGeneralCommand[9] = 194;
  bufferMqttGeneralCommand[10] = 0;
  bufferMqttGeneralCommand[11] = mqttAliveTime;
  bufferMqttGeneralCommand[12] = 0;
  bufferMqttGeneralCommand[13] = strlen(mqttClientID);
  for (uint16_t i = 0; i < strlen(mqttClientID); i++) {
    bufferMqttGeneralCommand[i + 14] = (uint8_t)mqttClientID[i];
  }
  bufferMqttGeneralCommand[strlen(mqttClientID) + 14] = 0;
  bufferMqttGeneralCommand[strlen(mqttClientID) + 15] = strlen(mqttUsername);
  for (uint16_t i = 0; i < strlen(mqttUsername); i++) {
    bufferMqttGeneralCommand[i + strlen(mqttClientID) + 16] = (uint8_t)mqttUsername[i];
  }
  bufferMqttGeneralCommand[strlen(mqttUsername) + strlen(mqttClientID) + 16] = 0;
  bufferMqttGeneralCommand[strlen(mqttUsername) + strlen(mqttClientID) + 17] = strlen(mqttPassword);
  for (uint16_t i = 0; i < strlen(mqttPassword); i++) {
    bufferMqttGeneralCommand[i + strlen(mqttUsername) + strlen(mqttClientID) + 18] = (uint8_t)mqttPassword[i];
  }
}
void generateStringMqttPublish(const char* mqttTopic, const char* mqttPayload) {
  memset(bufferMqttGeneralCommand, 0, BUFFER_MQTT_LENGTH);
  if (strlen(mqttTopic) + strlen(mqttPayload) + 2 > 127) {
    bufferMqttGeneralCommandLength = strlen(mqttTopic) + strlen(mqttPayload) + 5;
    bufferMqttGeneralCommand[0] = 48;
    bufferMqttGeneralCommand[1] = (uint8_t)((strlen(mqttTopic) + strlen(mqttPayload) + 2) % 128) + 128;
    bufferMqttGeneralCommand[2] = (uint8_t)((strlen(mqttTopic) + strlen(mqttPayload) + 2) / 128);
    bufferMqttGeneralCommand[3] = 0;
    bufferMqttGeneralCommand[4] = (uint8_t)strlen(mqttTopic);
    for (uint16_t i = 0; i < strlen(mqttTopic); i++) {
      bufferMqttGeneralCommand[i + 5] = (uint8_t)mqttTopic[i];
    }
    for (uint16_t i = 0; i < strlen(mqttPayload); i++) {
      bufferMqttGeneralCommand[i + 5 + strlen(mqttTopic)] = (uint8_t)mqttPayload[i];
    }
  }
  else {
    bufferMqttGeneralCommandLength = strlen(mqttTopic) + strlen(mqttPayload) + 4;
    bufferMqttGeneralCommand[0] = 48;
    bufferMqttGeneralCommand[1] = (uint8_t)(strlen(mqttTopic) + strlen(mqttPayload) + 2);
    bufferMqttGeneralCommand[2] = 0;
    bufferMqttGeneralCommand[3] = (uint8_t)strlen(mqttTopic);
    for (uint16_t i = 0; i < strlen(mqttTopic); i++) {
      bufferMqttGeneralCommand[i + 4] = (uint8_t)mqttTopic[i];
    }
    for (uint16_t i = 0; i < strlen(mqttPayload); i++) {
      bufferMqttGeneralCommand[i + 4 + strlen(mqttTopic)] = (uint8_t)mqttPayload[i];
    }
  }
}
void generateStringMqttSubscribe(const char* mqttTopic, bool subTrue) {
  memset(bufferMqttGeneralCommand, 0, BUFFER_MQTT_LENGTH);
  if (subTrue) {
    bufferMqttGeneralCommandLength = strlen(mqttTopic) + 6 + 1;
    bufferMqttGeneralCommand[0] = 130;
    bufferMqttGeneralCommand[1] = 5 + strlen(mqttTopic);
    bufferMqttGeneralCommand[2] = (uint8_t)(idMqttPacket / 256);
    bufferMqttGeneralCommand[3] = (uint8_t)(idMqttPacket % 256);
    bufferMqttGeneralCommand[4] = 0;
    //subscribe topic.length < 64
    bufferMqttGeneralCommand[5] = strlen(mqttTopic);
    for (uint16_t i = 0; i < strlen(mqttTopic); i++) {
      bufferMqttGeneralCommand[i + 6] = mqttTopic[i];
    }
    bufferMqttGeneralCommand[strlen(mqttTopic) + 6] = 0;//QOS=0
  }
  else {
    bufferMqttGeneralCommandLength = strlen(mqttTopic) + 6;
    bufferMqttGeneralCommand[0] = 162;
    bufferMqttGeneralCommand[1] = 4 + strlen(mqttTopic);
    bufferMqttGeneralCommand[2] = (uint8_t)(idMqttPacket / 256);
    bufferMqttGeneralCommand[3] = (uint8_t)(idMqttPacket % 256);
    bufferMqttGeneralCommand[4] = 0;
    //unsubscribe topic.length < 64
    bufferMqttGeneralCommand[5] = strlen(mqttTopic);
    for (int i = 0; i < strlen(mqttTopic); i++) {
      bufferMqttGeneralCommand[i + 6] = mqttTopic[i];
    }
  }
  idMqttPacket++;
}
