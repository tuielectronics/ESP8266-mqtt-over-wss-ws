#define SIZE_JSON_MQTT 512
void mqttMessageProcess(char* mqttTopic, char* mqttPayload, uint8_t sizeOfmqttTopic, uint16_t sizeOfmqttPayload) {
  SERIAL_DEBUG("[WSS CLIENT] message process for topic --> %s <--, size=%d; payload --> %s <--, size=%d\n", mqttTopic, sizeOfmqttTopic, mqttPayload, sizeOfmqttPayload);
  /*
   * parse JSON here
   */

}
