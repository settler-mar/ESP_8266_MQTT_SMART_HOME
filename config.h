void defaultConfig(){
  config.ssid = "Settler";
  config.password = "*******";
  config.mqtt_server = "192.168.0.108";
  config.temp_interval=10;
}

boolean loadConfig() {
  String out;
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJSON = jsonBuffer.parseObject(buf.get());

  if (!configJSON.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  const char *ssid=configJSON["ssid"];
  const char *password=configJSON["password"];
  WiFi.begin ( ssid, password);
//config.mqtt_server=StringToChar(configJSON["mqtt_server"]);
  //const char* serverName = configJSON["mqtt_server"];
  config.mqtt_server=clearString(configJSON["mqtt_server"]);
  config.ssid = StringToChar(configJSON["ssid"]);
  config.password = StringToChar(configJSON["password"]);
  config.temp_interval=StringToInt(configJSON["temp_interval"]);

  if(config.temp_interval<3)config.temp_interval=5;
  
  configJSON.prettyPrintTo(out);
  Serial.println(out);
  Serial.println("Config load");
  return true;
}


void saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJSON = jsonBuffer.createObject();
  configJSON["ssid"] = config.ssid;
  configJSON["password"] = config.password;
  configJSON["mqtt_server"] = config.mqtt_server;
  configJSON["temp_interval"] = config.temp_interval;
 
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  configJSON.printTo(configFile);
  Serial.println("Create new config");
}

void loadOneWere(){
  byte msk=255;
  Serial.println("Get DS data from config");
  byte addr[8][8];
  ds.reset_search();
  int i=0;
  while (ds.search(addr[i])) {
    if (OneWire::crc8(addr[i], 7) != addr[i][7]) {

    }else{
      i++;
    }
  }
  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/ds.json", "r");
  if (!configFile) {
    Serial.println("Failed to open ds file for reading");
    return;
  }
  while(configFile.available()) {
    String line = configFile.readStringUntil('\n');
    //Serial.println(line);
    JsonObject& root = jsonBuffer.parseObject(line);
    if (!root.success()) {
     Serial.println("parseObject() failed");
    }else{
      byte new_addr[8];
      for(int j=0;j<8;j++){
        new_addr[j]=root["addr"][j];
      }
      for(int j=0;j<i;j++){
        if(sootv_addr(addr[j],new_addr)){
          switch (new_addr[0]) {
            case 0x01:
              //"DS1990";
              break;
            case 0x10:
            case 0x28:
            case 0x22:
              for (byte n=0;n<8;n++){
               ds_temp[ds_temp_cnt].addr[n]=new_addr[n];
              };
              ds_temp[ds_temp_cnt].mqtt=root["mqtt"].as<String>();
              ds_temp_cnt++;
              break;
            case 0x85: //ds2413
            case 0x29:
              //option+="DS2408";
            case 0x12:
             //option+="DS24(06/07)";
              msk=255;
              if(new_addr[0]==0x85){
                msk=5;
              };
              if(new_addr[0]==0x12){
                msk=1;
              };
              for (byte n=0;n<8;n++){
               ds24_data[ds_24_cnt].addr[n]=new_addr[n];
              };
              ds24_data[ds_24_cnt].mqtt=root["mqtt"].as<String>();
              ds24_data[ds_24_cnt].role=StringToInt(root["role"].as<String>());
              ds24_data[ds_24_cnt].invert_=StringToInt(root["invert_"].as<String>());
              ds24_data[ds_24_cnt].start=StringToInt(root["start"].as<String>());             
              ds24_data[ds_24_cnt].last_satate=0;
              ds24_data[ds_24_cnt].last_out=ds24_data[ds_24_cnt].start;

              if(new_addr[0]==0x29 || new_addr[0]==0x85){
                if(~ds24_data[ds_24_cnt].role&msk){
                  String mqtt;
                  /*mqtt=ds24_data[ds_24_cnt].mqtt+"/*";
                  mqtt_add_out(mqtt);*/        
                  byte role=~ds24_data[ds_24_cnt].role&msk;
                  byte val=1;
                  for (int j=0;j<8;j++){
                    if(val & role){
                      mqtt=ds24_data[ds_24_cnt].mqtt+"/"+j;
                      mqtt_add_out(mqtt);
                    }
                    val=val*2;
                  }
                  
                }
                /*if(ds24_data[ds_24_cnt].role&0xFF){
                  byte val=~ds24_data[ds_24_cnt].role;
                  val=val|ds24_data[ds_24_cnt].role;
                  DS24_set(ds24_data[ds_24_cnt].addr,val);
                }*/
                byte val = ds24_data[ds_24_cnt].last_out;
                val = val ^ ds24_data[ds_24_cnt].invert_;
                DS24_set(ds24_data[ds_24_cnt].addr,val);
              }
              
              if(new_addr[0]==0x12){
                ds24_data[ds_24_cnt].last_out=0;
                if(ds24_data[ds_24_cnt].role==2){
                  mqtt_add_out(ds24_data[ds_24_cnt].mqtt);
                  if(ds24_data[ds_24_cnt].invert_){
                    DS24_set(ds24_data[ds_24_cnt].addr,0xFF);
                  }else{
                    DS24_set(ds24_data[ds_24_cnt].addr,0x00);
                  }
                }
              }
    
              ds_24_cnt++;
              break;
            case 0x26:
              for (byte n=0;n<8;n++){
               ds2438_data[ds_2438_cnt].addr[n]=new_addr[n];
              };
              ds2438_data[ds_2438_cnt].mqtt_t=root["mqtt_t"].as<String>();
              ds2438_data[ds_2438_cnt].mqtt_v=root["mqtt_v"].as<String>();
              ds2438_data[ds_2438_cnt].intrval=StringToInt(root["intrval"].as<String>());

              //init_2438(ds2438_data[ds_2438_cnt].addr);
              
              ds_2438_cnt++;
              break;
            case 0x20:
              for (byte n=0;n<8;n++){
               ds2450_data[ds_2450_cnt].addr[n]=new_addr[n];
              };
              ds2450_data[ds_2450_cnt].mqtt=root["mqtt"].as<String>();
              ds2450_data[ds_2450_cnt].intrval=StringToInt(root["intrval"].as<String>());

              //init_2450(ds2450_data[ds_2450_cnt].addr);
              ds_2450_cnt++;
              break;
          }
        }
      }
    }
  }
  Serial.print("DS_TEMP - ");
  Serial.println(ds_temp_cnt);

  Serial.print("DS2406 - ");
  Serial.println(ds_24_cnt);  

  Serial.print("DS2438 - ");
  Serial.println(ds_2438_cnt);  

  Serial.print("DS2450 - ");
  Serial.println(ds_2450_cnt); 
}

void saveOneWere() {
  Serial.println("Start save one wire...");
  File configFile = SPIFFS.open("/ds.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  
  DynamicJsonBuffer jsonBuffer;
  
  Serial.print("DS_TEMP - ");
  Serial.println(ds_temp_cnt);
  for (int i=0;i<ds_temp_cnt;i++){
    JsonObject& root = jsonBuffer.createObject();
    root["mqtt"] = ds_temp[i].mqtt.c_str();
    JsonArray& data = root.createNestedArray("addr");
    for (int j=0;j<8;j++){
      data.add(ds_temp[i].addr[j]);
    }
    root.printTo(configFile);
    configFile.println();
    //root.prettyPrintTo(Serial);
  }

  Serial.print("DS2406/08 - ");
  Serial.println(ds_24_cnt);
  for (int i=0;i<ds_24_cnt;i++){
    JsonObject& root = jsonBuffer.createObject();
    root["mqtt"] = ds24_data[i].mqtt.c_str();
    root["role"] = String(ds24_data[i].role);
    root["invert_"] = String(ds24_data[i].invert_);
    root["start"] = String(ds24_data[i].start);
    
    JsonArray& data = root.createNestedArray("addr");
    for (int j=0;j<8;j++){
      data.add(ds24_data[i].addr[j]);
    }
    root.printTo(configFile);
    configFile.println();
    //root.prettyPrintTo(Serial);
    //Serial.println();

    if(ds24_data[i].addr[0]==0x29 || ds24_data[i].addr[0]==0x85){
      byte mask=255;
      if(ds24_data[i].addr[0]==0x85){
        mask=5;
      }
      if(ds24_data[i].addr[0]==0x12){
        mask=1;
      }
      if(~ds24_data[i].role&mask){
        String mqtt;
        byte role=~ds24_data[i].role&mask;
        byte val=1;
        for (int j=0;j<8;j++){
          if(val & role){
            mqtt=ds24_data[i].mqtt+"/"+j;
            mqtt_add_out(mqtt);
          }
          val=val*2;
        }
      }
    }
    if(ds24_data[i].addr[0]==0x12){
      if(ds24_data[i].role==2){
        mqtt_add_out(ds24_data[i].mqtt);
      }
    }                  
  }

  Serial.print("DS2438 - ");
  Serial.println(ds_2438_cnt);
  for (int i=0;i<ds_2438_cnt;i++){
    JsonObject& root = jsonBuffer.createObject();
    root["mqtt_t"] = ds2438_data[i].mqtt_t.c_str();
    root["mqtt_v"] = ds2438_data[i].mqtt_v.c_str();
    root["intrval"] = ds2438_data[i].intrval;
    
    JsonArray& data = root.createNestedArray("addr");
    for (int j=0;j<8;j++){
      data.add(ds2438_data[i].addr[j]);
    }
    root.printTo(configFile);
    configFile.println();
    //root.prettyPrintTo(Serial);
    //Serial.println();    
  }

  Serial.print("DS2450 - ");
  Serial.println(ds_2450_cnt);
  for (int i=0;i<ds_2450_cnt;i++){
    JsonObject& root = jsonBuffer.createObject();
    root["mqtt"] = ds2450_data[i].mqtt.c_str();
    root["intrval"] = ds2450_data[i].intrval;
    
    JsonArray& data = root.createNestedArray("addr");
    for (int j=0;j<8;j++){
      data.add(ds2450_data[i].addr[j]);
    }
    root.printTo(configFile);
    configFile.println();
    //root.prettyPrintTo(Serial);
    //Serial.println();    
  }
  mqtt_subscribe();
  configFile.close();
}
