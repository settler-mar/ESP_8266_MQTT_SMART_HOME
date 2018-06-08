void handleNotFound() {
  if(handleFileRead(server.uri())) return;
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  message +="=================";

  server.send ( 404, "text/plain", message );
}

void indexPage(){
  handleFileRead("/index.html");
}

#ifdef web_reboot
  void softRestart(){
    server.send ( 200, "text/html", "reset...." );
    ESP.reset();
  }
#endif
  
void main_config(){
  if (server.hasArg("mqtt_server")){
    config.ssid = server.arg("ssid");
    config.password = server.arg("password");
    config.mqtt_server = server.arg("mqtt_server");
    config.temp_interval = StringToInt(server.arg("temp_interval"));
    if(config.temp_interval<3){
      config.temp_interval=3;
    }
    config.mqtt_user = clearString(server.arg("mqtt_user"));
    config.mqtt_pass = clearString(server.arg("mqtt_pass"));
    config.mqtt_name = clearString(server.arg("mqtt_name"));

    config.update_dir = clearString(server.arg("update_dir"));
    config.update_file = clearString(server.arg("update_file"));
    
    saveConfig();
  }
  
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return ;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJSON = jsonBuffer.parseObject(buf.get());

  if (!configJSON.success()) {
    Serial.println("Failed to parse config file");
    return;
  }
  
  String out;
  configJSON.printTo(out);
  server.send ( 200, "text/html", out );
  
  return;
}

void nav_menu(){
  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();

  #ifdef ONE_WIRE_PORT
  {
    JsonObject& data = root.createNestedObject();
    data["title"]="OneWire";
    data["url"]="/#one-wire";
  }
  #endif
  
  #ifdef DHT11_PIN
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="DHT 11";
      data["url"]="/#dht_11";
    }
  #endif
  #ifdef BMP180
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="BMP180";
      data["url"]="/#bmp180";
    }
  #endif
  #ifdef BME_280
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="BME280";
      data["url"]="/#bme280";
    }
  #endif
  #ifdef analog_pin
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="Analog";
      data["url"]="/#analog";
    }
  #endif

  #ifdef RDM6300
   {
      JsonObject& data = root.createNestedObject();
      data["title"]="RDM6300";
      data["url"]="/#rdm6300";
    }
  #endif
  
  #ifdef PID_TEMP
      JsonObject& data = root.createNestedObject();
      data["title"]="PID temperature";
      data["url"]="/#pid_temp";
  #endif

  #ifdef ws2812_run
      JsonObject& data = root.createNestedObject();
      data["title"]="WS RUN";
      data["url"]="/#ws-run";
  #endif
  
  #ifdef MCP
      JsonObject& data = root.createNestedObject();
      data["title"]="mcp23017";
      data["url"]="/#mcp23017";
  #endif

  #ifdef PCA
      JsonObject& data_pca = root.createNestedObject();
      data_pca["title"]="pca9685";
      data_pca["url"]="/#pca9685";
  #endif


  #ifdef RC433_PORT
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="RC433";
      data["url"]="/#rc433";
    }
  #endif

  #ifdef WS_PIN
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="WS2812FX";
      data["url"]="/#ws2812fx";
    }
  #endif
    
  #ifdef FILE_EDIT
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="EDITOR";
      data["url"]="/edit.htm";
    }
  #endif

  
  #ifdef wifi_ota
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="ON OTA";
      data["url"]="/wifi_ota";
    }
   #endif
   #ifdef wifi_update
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="UPDATE FROM NET";
      data["url"]="/auto_update";
    }
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="UPDATE spiffs";
      data["url"]="/auto_update_spiffs";
    }
  #endif

  #ifdef web_reboot
  {
    JsonObject& data = root.createNestedObject();
    data["title"]="restart";
    data["url"]="/restart";
  }
  #endif
  
  File configFile = SPIFFS.open("/nav.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  root.printTo(configFile);
  Serial.println("Create new menu (nav.json)");
  /*String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );*/
}

#ifdef wifi_ota
  void wifi_ota_init(){
    wifi_ota_run=true;
    server.send ( 200, "text/html", "wifi_ota RUN" );
    return;
  }

#endif
#ifdef wifi_update
  void auto_update_init(){
    //ESPhttpUpdate
    Serial.println(config.update_dir+"/"+config.update_file+".bin");
    t_httpUpdate_return ret = ESPhttpUpdate.update(config.update_dir+"/"+config.update_file+".bin");
    //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        server.send ( 200, "text/html", "auto_update FAILED" );
        Serial.println();
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        server.send ( 200, "text/html", "auto_update NO_UPDATES" );
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        server.send ( 200, "text/html", "auto_update OK" );
        break;
    }
  }
  
  void auto_update_spiffs_init(){
    //ESPhttpUpdate
    if(!downloadFile(config.update_dir+"/spiffs/file_list.data","file_list.data")){
      server.send ( 200, "text/html", "update file list FAILED" );
      Serial.println("update file list FAILED");
      return;
    }

    File configFile = SPIFFS.open("/file_list.data", "r");
    if (!configFile) {
      server.send ( 200, "text/html", "Failed to open file list for reading" );
      Serial.println("Failed to open file list for reading");
      return;
    }
    while(configFile.available()) {
      String line = configFile.readStringUntil('\n');
      downloadFile(config.update_dir+"/spiffs/"+line,line);
    }
    server.send ( 200, "text/html", "update - finish" );
  }
#endif

void main_info(){
  String out="Mhome\n";
  out+=(String)VERSION+"\n";
  out+=config.mqtt_name+"\n";
  out+=(String)ESP.getChipId()+"\n";
  out+=(String)ESP.getFlashChipRealSize()+"\n";
  out+=(String)ESP.getFlashChipSpeed()+"\n";
  out+=(String)ESP.getVcc()+"\n";
  out+=(String)WiFi.SSID()+"\n";
  out+=(String)WiFi.RSSI()+"\n";
  
  server.send ( 200, "text/html", out);
}

void firmware_config(){
  String out="";

  //out+=(String)VERSION+"\n";
  //out+=config.mqtt_name;
  #ifdef DEBUG_ENABLE
  out+="#define DEBUG_ENABLE\n";
  #endif
  #ifdef ESP12_pins
  out+="#define ESP12_pins\n";
  #endif
  #ifdef wifi_ota
  out+="#define wifi_ota\n";
  #endif
  #ifdef wifi_update
  out+="#define wifi_update\n";
  #endif

  #ifdef web_reboot
  out+="#define web_reboot\n";
  #endif
  #ifdef FILE_EDIT
  out+="#define FILE_EDIT\n";
  #endif
  #ifdef UN_drebizg
  out+="#define UN_drebizg "+(String)UN_drebizg+"\n";
  #endif
  #ifdef T_PERIOD
  out+="#define T_PERIOD "+(String)T_PERIOD+"\n";
  #endif
  #ifdef UART_SPEAD
  out+="#define UART_SPEAD "+(String)UART_SPEAD+"\n";
  #endif
  #ifdef pulse_perion
  out+="#define pulse_perion "+(String)pulse_perion+"\n";
  #endif
  #ifdef ESP_auth_def
  out+="#define ESP_auth_def\n";
  #endif


  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return ;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJSON = jsonBuffer.parseObject(buf.get());

  if (!configJSON.success()) {
    Serial.println("Failed to parse config file");
    return;
  }

  out+="#define WIFI_SSID \""+clearString(configJSON["ssid"])+"\"\n";
  out+="#define WIFI_PASSWORD \""+clearString(configJSON["password"]) +"\"\n";
  out+="#define MQTT_NAME \""+config.mqtt_name+"\"\n";
  out+="#define UPDATE_DIR \""+config.update_dir+"\"\n";
  out+="#define UPDATE_FILE \""+config.update_file+"\"\n";
  out+="#define WWW_LOGIN \""+String(config.www_login)+"\"\n";
  out+="#define WWW_PASS \""+String(config.www_password)+"\"\n";
  
  #ifdef ONE_WIRE_PORT
  out+="#define ONE_WIRE_PORT "+(String)ONE_WIRE_PORT+"\n";
    #ifdef DS_1820_personal_convert
      out+="#define DS_1820_personal_convert\n";
    #endif
  #endif
  
  #ifdef DHT11_PIN
  out+="#define DHT11_PIN "+(String)DHT11_PIN+"\n";
    #ifdef DHTTYPE
    out+="#define DHTTYPE "+(String)DHTTYPE+"\n";
    #endif
  #endif
  
  #ifdef BMP180
  out+="#define BMP180 "+(String)BMP180+"\n";
  #endif
  #ifdef BME_280
    out+="#define BME_280 "+(String)BME_280+"\n";
    #ifdef BME_280_ADDR
      out+="#define BME_280_ADDR "+(String)BME_280_ADDR+"\n";
    #endif
  #endif
  
  #ifdef analog_pin
  out+="#define analog_pin "+(String)analog_pin+"\n";
  #endif

  #ifdef MCP
      out+="#define MCP "+(String)MCP+"\n";
  #endif

  #ifdef PCA
      out+="#define PCA "+(String)PCA+"\n";
  #endif

  #ifdef RC433_PORT
    out+="#define RC433_PORT "+(String)RC433_PORT+"\n";
  #endif

  
  #ifdef RDM6300
    out+="#define RDM6300\n";
    #ifdef RDM6300_APARAT_UART
    out+="#define RDM6300_APARAT_UART\n";
    #endif
     #ifdef RDM6300_SOFT_UART_RX
    out+="#define RC433_PORT "+(String)RDM6300_SOFT_UART_RX+"\n";
    #endif
    #ifdef RDM6300_SOFT_UART_TX
    out+="#define RC433_PORT "+(String)RDM6300_SOFT_UART_TX+"\n";
    #endif
  #endif

  #ifdef PID_TEMP
    out+="#define PID_TEMP\n";
  #endif

  #ifdef ws2812_run 
    out+="#define ws2812_run\n";
    out+="#define PIN_ws "+(String)PIN_ws+"\n";
    out+="#define NUMPIXELS "+(String)NUMPIXELS+"\n";
    #ifdef ws2812_run_up_pin
    out+="#define ws2812_run_up_pin "+(String)ws2812_run_up_pin+"\n";
    #endif
    #ifdef ws2812_run_dwn_pin
    out+="#define RC433_PORT "+(String)ws2812_run_dwn_pin+"\n";
    #endif
    #ifdef ws2812_run_analog_pin
    out+="#define ws2812_run_analog_pin "+(String)ws2812_run_analog_pin+"\n";
    #endif
  #endif
  
  #ifdef WS_PIN
    out+="#define WS_PIN "+(String)WS_PIN+"\n";
    out+="#define WS_LED_COUNT "+(String)WS_LED_COUNT+"\n";
  #endif

  
  server.send ( 200, "text/html", out);
}


void HTTP_route(){
  String uri = server.uri();
  int method = server.method();
  //String ip = 

  #ifdef ESP_auth_def
   if (!is_authentified()){
      loginRoute();
      return;
    }
  #endif
    
  if(uri == "/main")return main_config ();
  if(uri == "/info")return main_info ();
  if(uri == "/firmware_config")return firmware_config ();
  if(uri == "/networks")return listNetworks ();

  #ifdef ONE_WIRE_PORT
    if(uri == "/one-wire")return ds_list_find ();
    if(uri == "/ds1820")return ds18b20_config();
    if(uri == "/ds2406")return ds2406_config();
    if(uri == "/ds2408")return ds2408_config();
    if(uri == "/ds2438")return ds2438_config();
    if(uri == "/ds2450")return ds2450_config();
  #endif

  #ifdef web_reboot
    if(uri == "/restart")return softRestart();
  #endif
  //if(uri == "/nav.json")return nav_menu);

  #ifdef DHT11_PIN
    if(uri == "/dht11")return dht11_config();
  #endif
  #ifdef BMP180
    if(uri == "/bmp180")return bmp180_config();
  #endif
  #ifdef BME_280
    if(uri == "/bme280")return bme280_config();
  #endif
  #ifdef analog_pin
    if(uri == "/analog")return analog_config();
  #endif
  #ifdef RDM6300
   if(uri == "/rdm6300")return rdm6300_config();
  #endif

  #ifdef PID_TEMP
     if(uri == "/pid_temp")return pid_temp_config();
  #endif

  #ifdef ws2812_run
      if(uri == "/ws_run")return ws_run_config();
  #endif
  
  #ifdef FILE_EDIT
    //list directory
    if(uri == "/list" && method==HTTP_GET)return handleFileList();
    //load editor
    if(uri == "/edit" && method==HTTP_GET){
      if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
      return;
    };
    //create file
    if(uri == "/edit" && method==HTTP_PUT)return handleFileCreate();
    //delete file
    if(uri == "/edit" && method==HTTP_DELETE) return handleFileDelete();
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    /*if(uri == "/edit" && method==HTTP_POST){ 
      handleFileUpload();
      server.send(200, "text/plain", ""); 
      return;
    }*/
  #endif
  

  #ifdef wifi_ota
    if(uri == "/wifi_ota")return wifi_ota_init();
  #endif
  
  #ifdef wifi_update
    if(uri == "/auto_update")return auto_update_init();
    if(uri == "/auto_update_spiffs")return auto_update_spiffs_init();
  #endif

  #ifdef RC433_PORT
    if(uri == "/rc433")return rc433_config();
  #endif

  #ifdef WS_PIN
    if(uri == "/ws2812fx")return ws2812fx_config();
  #endif

  #ifdef MCP
    if(uri == "/mcp23017")return mcp23017_config();
  #endif

  #ifdef PCA
    if(uri == "/pca9685")return pca9685_config();
  #endif

  handleNotFound ();
}

void server_init(){
  nav_menu();
  
  server.on ( "/", indexPage );

  #ifdef FILE_EDIT
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
  #endif
  
  server.onNotFound ( HTTP_route );
  server.begin();
  Serial.println ( "HTTP server started" );
}

