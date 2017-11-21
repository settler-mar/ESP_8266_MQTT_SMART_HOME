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

void softRestart(){ESP.reset();}
void main_config(){
  if (server.hasArg("mqtt_server")){
    config.ssid = server.arg("ssid");
    config.password = server.arg("password");
    config.mqtt_server = server.arg("mqtt_server");
    config.temp_interval = StringToInt(server.arg("temp_interval"));
    if(config.temp_interval<3){
      config.temp_interval=3;
    }
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
  {
    JsonObject& data = root.createNestedObject();
    data["title"]="OneWire";
    data["url"]="/#one-wire";
  }
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
  
  #ifdef FILE_EDIT
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="EDITOR";
      data["url"]="/edit.htm";
    }
  #endif

  #ifdef wifi_firmware_update
    {
      JsonObject& data = root.createNestedObject();
      data["title"]="update";
      data["url"]="/update";
    }
  #endif
  

  /*  {
      JsonObject& data = root.createNestedObject();
      data["title"]="restart";
      data["url"]="/restart";
    }*/

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

void server_init(){
  nav_menu();
  
  server.on ( "/", indexPage );
  server.on ( "/main", main_config );
  server.on ( "/one-wire", ds_list_find );
  server.on ( "/ds1820",ds18b20_config);
  server.on ( "/ds2406",ds2406_config);
  server.on ( "/ds2408",ds2408_config);
  server.on ( "/ds2438",ds2438_config);
  server.on ( "/ds2450",ds2450_config);
    
  server.on ( "/restart",softRestart);
  //server.on ( "/nav.json", nav_menu);

  #ifdef DHT11_PIN
    server.on ( "/dht11",dht11_config);
  #endif
  #ifdef BMP180
    server.on ( "/bmp180",bmp180_config);
  #endif
  #ifdef analog_pin
    server.on ( "/analog",analog_config);
  #endif
  #ifdef RDM6300
   server.on ( "/rdm6300",rdm6300_config);
  #endif

  #ifdef PID_TEMP
     server.on ( "/pid_temp",pid_temp_config);
  #endif

  #ifdef ws2812_run
      server.on ( "/ws_run",ws_run_config);
  #endif
  
  #ifdef FILE_EDIT
    //list directory
    server.on("/list", HTTP_GET, handleFileList);
    //load editor
    server.on("/edit", HTTP_GET, [](){
      if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
    });
    //create file
    server.on("/edit", HTTP_PUT, handleFileCreate);
    //delete file
    server.on("/edit", HTTP_DELETE, handleFileDelete);
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
  #endif
  
  #ifdef wifi_firmware_update
    httpUpdater.setup(&server);
  #endif

  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}

