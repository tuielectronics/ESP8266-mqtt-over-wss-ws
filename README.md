# ESP8266-mqtt-over-wss-ws
> Arduino ESP8266 mqtt client over WebSockets layer and WebSockets secure without 3rd-party library

## Step 1, 
> choose WebSockets or WebSockets secure (TLS), if you choose USE_TLS, please update your root CA 

`#define USE_TLS 1 //comment this line to disable TLS`

## Step 2, 
> configure WiFi
```javascript
const char* wifiPassword = "my_password"; 
const char* wifiSsid = "my_name";
```
## Step 3, 
> configure MQTT
```javascript
const char* wssHost = "m23.cloudmqtt.com"; 
const char* wssPath = "/"; int wssPort = 38996; 
const char* MQTTUsername = "aizzyznu"; 
const char* MQTTPassword = "f2Eyph2tS889";
```

