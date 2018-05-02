Adafruit_BMP085 bmp;

struct bmp_data {
  String mqtt_temp;
  String mqtt_pre;
  boolean active;
} bmp_185;

void BMP180_init(){
  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/bmp180.json", "r");
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
     bmp_185.mqtt_temp = root["mqtt_temp"].as<String>();
     bmp_185.mqtt_pre = root["mqtt_pre"].as<String>();
     bmp_185.active = true;
  }

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
}


void BMP180_read(){
  float pa;

  if(bmp_185.active){
    mqtt_pub(bmp_185.mqtt_temp,String(bmp.readTemperature()));

    /*Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");*/

    //bmp.readTemperature() //температура в цельсиях

    //Serial.print("Pressure = ");
    pa = (float)bmp.readPressure();
    #ifdef BMP180 == 2
      pa=pa/133.322;
    #endif

    mqtt_pub(bmp_185.mqtt_pre,String(pa));
    /*Serial.print(pa);
    Serial.println("");*/
  }
}

void bmp180_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_pre")){
     bmp_185.mqtt_temp = server.arg("mqtt_temp");
     bmp_185.mqtt_pre = server.arg("mqtt_pre");
     bmp_185.active = true;
  }

  root["mqtt_temp"] = bmp_185.mqtt_temp;
  root["mqtt_pre"]  = bmp_185.mqtt_pre;

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_pre")){
    File configFile = SPIFFS.open("/bmp180.json", "w");
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
