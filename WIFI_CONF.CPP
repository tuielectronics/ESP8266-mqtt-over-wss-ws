void wifiInit() {
  WiFi.mode(WIFI_STA);

  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    SERIAL_DEBUG("->");
    delay(1000);
  }
  SERIAL_DEBUG("[WiFi] Connected. IP is %s\n", WiFi.localIP().toString().c_str());
}

void wifiCheck() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiInit();
  }
}
