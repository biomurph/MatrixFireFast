
String getFlariables(){
  JsonDocument kindling;
  kindling["isBurning"] = isBurning;
  kindling["flareRows"] = flareRows;
  kindling["flareMax"] = flareMax;
  kindling["flareChance"] = flareChance;
  kindling["flareDecay"] = flareDecay;
  kindling["meta"] = versionString;

  serializeJson(kindling, gasoline);
  Serial.print("gas "); Serial.println(gasoline);
  return gasoline;
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(F("Connecting to WiFi .."));
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  if(!MDNS.begin(hostname)) {
     Serial.println("Error starting mDNS");
  }
  Serial.print(F("\nOpen the page at "));
  Serial.print(WiFi.localIP());Serial.write('\t'); Serial.print(hostname); Serial.println(F(".local"));
  Serial.print("RSSI "); Serial.println(WiFi.RSSI());
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;  // terminate the string
    if (strcmp((char*)data, "getFlariables") == 0) {
      notifyClients(getFlariables());
    } else {
      for(int i=0; i<=len; i++){
        msg[i] = char(data[i]);
      }
      Serial.print("msg "); Serial.println(msg);
      JsonDocument flariables;
      DeserializationError error = deserializeJson(flariables, msg);
      if (error){ //Check for errors in parsing
        Serial.print(F("Parsing failed"));  //  at "); Serial.println(error); ??
        return;
      }
      isBurning = flariables["isBurning"];
      flareRows = flariables["flareRows"];
      flareMax = flariables["flareMax"];
      flareChance = flariables["flareChance"];
      flareDecay = flariables["flareDecay"];
      gotUpdated = true;
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break; }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}


void notifyClients(String state) {
  ws.textAll(state);
}