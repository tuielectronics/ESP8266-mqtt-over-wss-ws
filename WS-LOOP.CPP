#define WS_HEARTBEAT_INTERVAL 10000
#define WS_MQTT_HEARTBEAT_INTERVAL 10000
static long timeLastWsConnect = 0;
static long timeLastWsHeartBeat = 0;
static long timeLastMqttHeartBeat = 0;
static long timeLastMqttPublish = 0;
void wssConnect() {
  if (wsStatus != WS_NOT_START) {
    SERIAL_DEBUG("[WSS CLIENT] wssConnect not ready yet...\n");
    return;
  }
#ifdef USE_TLS
  setClock();
  BearSSL::X509List cert(digicert);
  sClient.setTrustAnchors(&cert);
  SERIAL_DEBUG("[WSS CLIENT] ws connect with TLS ...\n");
  
  if (!sClient.connect(wssHost, wssPort)) {
    SERIAL_DEBUG("[WSS CLIENT] failed to connect to %s:%d\n", wssHost, wssPort);
    delay(5000);
    return;
  }
  
  //  if (!sClient.verifyCertChain(wssHost)) {
  //    Serial.println("[WSS CLIENT] certificate verification failed!\n");
  //    delay(5000);
  //    return;
  //  }
#else
  SERIAL_DEBUG("[WSS CLIENT] ws connect without TLS ...\n");
  if (!sClient.connect(wssHost, wssPort)) {
    SERIAL_DEBUG("[WSS CLIENT] failed to connect to %s:%d\n", wssHost, wssPort);
    delay(5000);
    return;
  }
#endif
  SERIAL_DEBUG("[WSS CLIENT] connected to %s:%d\n", wssHost, wssPort);

  wsKeyClient = "KR/" + String(millis()) + "/" + WiFi.macAddress() + "==";
  String wsHeader = "";
  wsHeader += "GET " + String(wssPath) + " HTTP/1.1\r\n";
  wsHeader += "Upgrade: websocket\r\n";
  wsHeader += "Connection: Upgrade\r\n";
  wsHeader += "Host: " + String(wssHost) + "\r\n";
  wsHeader += "Origin: http://" + String(wssHost) + "\r\n";
  wsHeader += "Sec-WebSocket-Key: ";
  wsHeader += wsKeyClient;
  wsHeader += "\r\n";
  wsHeader += "Sec-WebSocket-Version: 13\r\n";
  wsHeader += "\r\n";
  SERIAL_DEBUG("[WSS CLIENT] send: %s\n", wsHeader.c_str());
  sClient.print(wsHeader);
  wsStatus = WS_GET_SENT;
  timeLastWsConnect = millis();
}
void wssMainLoop(long timeFromMain) {

  if (wsStatus == WS_NOT_START) {
    wssConnect();
  }
  else if (wsStatus > WS_NOT_START) {
    if (sClient.connected()) {
      int wsRecLength = sClient.available();
      if (wsRecLength > 0) {
        SERIAL_DEBUG("[WSS CLIENT] get ws data, length=%d\n", wsRecLength);
        if (wsStatus < WS_HANDSHAKE_FINISHED) {
          String line = sClient.readStringUntil('\n');
          Serial.println(line);
          if (line.startsWith("HTTP/1.1 101 Switching Protocols") && wsStatus == WS_GET_SENT) {
            wsStatus = WS_GET_HEADER_SWITCH;
            SERIAL_DEBUG("[WSS CLIENT] get websocket upgrade from server, delay = %d mS.\n", timeFromMain - timeLastWsConnect);
          }
          if (line.startsWith("Sec-WebSocket-Accept") && wsStatus == WS_GET_HEADER_SWITCH) {
            int i = line.indexOf(": ");
            if (i > 0 && i < line.length() - 3) {
              wsStatus = WS_HANDSHAKE_FINISHED;
              timeLastWsHeartBeat = timeFromMain;
              //verifyWsKey(line.substring(i + 2));
              String key = line.substring(i + 2);
              SERIAL_DEBUG("[WSS CLIENT] get Key from server respond: %s\n", key.c_str());
            }
          }
        }
        else if (wsStatus >= WS_HANDSHAKE_FINISHED) {
          if ( wsRecLength <= WS_BUFFER_LENGTH) {
            sClient.read(wsBufferRead, wsRecLength);

            if (wsBufferRead[0] == WS_KEYWORD_V_PONG) {
              if ( wsBufferRead[1] == 3 && wsBufferRead[2] == wsPingStr[6] && wsBufferRead[3] == wsPingStr[7] && wsBufferRead[4] == wsPingStr[8]) {
                SERIAL_DEBUG("[WSS CLIENT] ws pong received, delay= %d mS\n", timeFromMain - timeLastWsHeartBeat);
              }
            }
            else if (wsBufferRead[0] == WS_KEYWORD_V_BIN) {
              SERIAL_DEBUG("[WSS CLIENT] get BIN ->");
              for (uint16_t i = 0; i < wsRecLength; i++) {
                SERIAL_DEBUG("0x%02X,", wsBufferRead[i]);
              }
              SERIAL_DEBUG("\n");
              // check 126
              uint8_t offsetWs = wsRecLength < (126 + 2) ? 0 : 2;
              bool dataVerified = (offsetWs == 0 && (wsBufferRead[1] + 2) == wsRecLength) || (offsetWs == 2 && (uint16_t(wsBufferRead[2]) * 256 + uint16_t(wsBufferRead[3]) + 4) == wsRecLength);
              // veryfy if data is complete
              if (dataVerified) {
                SERIAL_DEBUG("[WSS CLIENT] BIN data verified\n");
                if (wsStatus == WS_MQTT_CONNECT_SENT) {
                  if ( wsBufferRead[1] == 4 && wsBufferRead[2] == WS_KEYWORD_V_MQTT_CONNECTED_BIT1 && wsBufferRead[3] == WS_KEYWORD_V_MQTT_CONNECTED_BIT2) {
                    wsStatus = WS_MQTT_CONNECTED;
                    timeLastMqttHeartBeat = timeFromMain;
                    SERIAL_DEBUG("[MQTT] MQTT connected.\n");
                    // do something here example

                    wsSendMqttSubUnsub("to/test/123", true);
                    wsSendMqttSubUnsub("to/test1/123", true);
                  }
                }
                else if (wsStatus == WS_MQTT_CONNECTED) {
                  if (wsBufferRead[1] > 4 && wsBufferRead[2 + offsetWs] == WS_KEYWORD_V_MQTT_RECEIVE) {
                    uint16_t mqttBodySize = wsRecLength - 2 - offsetWs;
                    uint8_t offsetMqtt = mqttBodySize <= (127 + 2) ? 0 : 1;
                    uint8_t mqttTopicSize = wsBufferRead[2 + offsetWs + offsetMqtt + 3];
                    uint16_t mqttTopicIndex = 2 + offsetWs + offsetMqtt + 4;
                    uint16_t mqttPayloadSize = wsRecLength - mqttTopicIndex - mqttTopicSize;
                    uint16_t mqttPayloadIndex = mqttTopicIndex + mqttTopicSize;

                    char mqttTopic[mqttTopicSize + 1];
                    char mqttPayload[mqttPayloadSize + 1];
                    mqttTopic[mqttTopicSize] = '\0'; // must add this to mark the end of String
                    mqttPayload[mqttPayloadSize] = '\0';
                    memcpy(mqttTopic, &wsBufferRead[mqttTopicIndex], mqttTopicSize);
                    memcpy(mqttPayload, &wsBufferRead[mqttPayloadIndex], mqttPayloadSize);

                    // process received mqtt message
                    mqttMessageProcess(mqttTopic, mqttPayload, mqttTopicSize, mqttPayloadSize);

                  }
                  else if ( wsBufferRead[1] == 5 && wsBufferRead[2] == WS_KEYWORD_V_MQTT_SUBACK) {
                    if (wsBufferRead[6] == WS_KEYWORD_V_MQTT_SUBFAIL) {
                      SERIAL_DEBUG("[MQTT] failed to subscribe to packet id = %d\n", wsBufferRead[4] * 256 + wsBufferRead[5]);
                    }
                    else {
                      SERIAL_DEBUG("[MQTT] subscribed to packet id = %d, QOS = %d\n", wsBufferRead[4] * 256 + wsBufferRead[5], wsBufferRead[6]);
                    }
                  }
                  else if ( wsBufferRead[1] == 4 && wsBufferRead[2] == WS_KEYWORD_V_MQTT_UNSUBACK) {
                    SERIAL_DEBUG("[MQTT] unsubscribed to packet id = %d\n", wsBufferRead[4] * 256 + wsBufferRead[5]);
                  }
                  else if ( wsBufferRead[1] == 2 && wsBufferRead[2] == WS_KEYWORD_V_MQTT_PONG_BIT1 && wsBufferRead[3] == WS_KEYWORD_V_MQTT_PONG_BIT2) {
                    SERIAL_DEBUG("[MQTT] pong received, delay= %d mS\n", timeFromMain - timeLastMqttHeartBeat);
                  }
                  else {
                    SERIAL_DEBUG("[MQTT] unknown -> %d\n", wsRecLength);
                  }
                }
              }
              else {
                SERIAL_DEBUG("[WSS CLIENT] BIN data verify failed\n");
              }
            }
            memset(wsBufferRead, 0, wsRecLength);
          }
          else {
            uint8_t temp[wsRecLength];
            sClient.read(temp, wsRecLength);
            SERIAL_DEBUG("[WSS CLIENT] get too much data.");
          }
        }
      }
      else {
        if (timeFromMain - timeLastWsHeartBeat >= WS_HEARTBEAT_INTERVAL && wsStatus >= WS_HANDSHAKE_FINISHED) {
          wsSendHeartBeat();

          timeLastWsHeartBeat = timeFromMain;
        }
        else if (timeFromMain - timeLastMqttHeartBeat >= WS_MQTT_HEARTBEAT_INTERVAL && wsStatus >= WS_MQTT_CONNECTED) {
          wsSendMqttHeartBeat();
          timeLastMqttHeartBeat = timeFromMain;
        }
        else if (wsStatus == WS_HANDSHAKE_FINISHED) {
          wsStatus = WS_MQTT_CONNECT_SENT;
          wsSendMqttConnect();
        }

        if (wsStatus == WS_MQTT_CONNECTED) {

          // do something example
          if (timeFromMain - timeLastMqttPublish >= 5000) {
            timeLastMqttPublish = timeFromMain;

            wsSendMqttPublish("to/test/123", "{\"t\":1}");

          }
        }

      }
    }
    else {
      wsStatus = WS_NOT_START;
      SERIAL_DEBUG("[WSS CLIENT] ws disconnected at %d\n", millis());
      delay(2500);
      wifiCheck();
    }
  }
}
