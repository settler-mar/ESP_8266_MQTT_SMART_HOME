String analog_mqtt;
boolean analog_active;
int analog_intrval;

void analog_read(){
  if(analog_active){
     int smoke_level= analogRead(analog_pin); // считать уровень дыма
     //Serial.println(String(smoke_level));
     //Serial.println(analog_mqtt);
     mqtt_pub(analog_mqtt,String(smoke_level));
  }
}


void analog_init(){
  pinMode(analog_pin, INPUT);
  DynamicJsonBuffer jsonBuffer;

  analog_intrval=3;
  analog_mqtt="/home/room1/analog_test";
  File configFile = SPIFFS.open("/analog.json", "r");
  if (!configFile) {
    Serial.println("Failed to open ANALOG config file for reding");
    return;
  }
  String line = configFile.readStringUntil('\n');
    //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
  }else{
     analog_mqtt = root["mqtt"].as<String>();
     analog_intrval=StringToInt(root["intrval"].as<String>());
     if(analog_intrval<3)analog_intrval=3;
     analog_active = true;    
  }
}


void analog_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt")){
     analog_mqtt = server.arg("mqtt");
     analog_intrval = StringToInt(server.arg("intrval"));
     if(analog_intrval<3)analog_intrval=3;
     analog_active = true;
  }
  
  root["mqtt"] = analog_mqtt;
  root["intrval"]=analog_intrval;

  if (server.hasArg("mqtt")){
    File configFile = SPIFFS.open("/analog.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return;
    }
    root.printTo(configFile);
    configFile.close();
    
    root["msg"]="Data updated.";
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

