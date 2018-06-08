struct bme_data {
  String mqtt_temp;
  String mqtt_pre;
  String mqtt_hum;
  boolean active;
} bme_280;

Adafruit_BME280 bme; // I2C

void BME280_init(){
  #ifdef BME_280_ADDR
    if (!bme.begin(BME_280_ADDR)) {
      Serial.println("1 Could not find a valid BME280 sensor, check wiring! ");
      bme_280.active = false;
    }
  #else
    if (!bme.begin()) {
      Serial.println("0 Could not find a valid BME280 sensor, check wiring!");
      bme_280.active = false;
    }
  #endif
  
  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/bme280.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }
  String line = configFile.readStringUntil('\n');
    //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
  }else{
     bme_280.mqtt_temp = root["mqtt_temp"].as<String>();
     bme_280.mqtt_pre = root["mqtt_pre"].as<String>();
     bme_280.mqtt_hum = root["mqtt_hum"].as<String>();
     bme_280.active = true;
  }
}

void BME280_read(){
  float pa;

  if(bme_280.active){
    mqtt_pub(bme_280.mqtt_temp,String(bme.readTemperature()));
    mqtt_pub(bme_280.mqtt_hum,String(bme.readHumidity()));

    pa = (float)bme.readPressure();
    if( BME_280 == 2){
      pa=pa/133.322;
    }
    mqtt_pub(bme_280.mqtt_pre,String(pa));
  }
}

void bme280_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_pre")){
     bme_280.mqtt_temp = server.arg("mqtt_temp");
     bme_280.mqtt_pre = server.arg("mqtt_pre");
     bme_280.mqtt_hum = server.arg("mqtt_hum");
     bme_280.active = true;
  }

  root["mqtt_temp"] = bme_280.mqtt_temp;
  root["mqtt_pre"]  = bme_280.mqtt_pre;
  root["mqtt_hum"]  = bme_280.mqtt_hum;

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_pre")){
    File configFile = SPIFFS.open("/bme280.json", "w");
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
