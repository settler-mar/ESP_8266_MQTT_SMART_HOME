DHT_Unified dht(DHT11_PIN, DHTTYPE);

//ds temp
struct dht_data {
  String mqtt_temp;
  String mqtt_hum;
  boolean active;
} dht_11;


void DHT_init(){
  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/dht.json", "r");
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
     dht_11.mqtt_temp = root["mqtt_temp"].as<String>();
     dht_11.mqtt_hum = root["mqtt_hum"].as<String>();
     dht_11.active = true;    
  }
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
/* Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.*/
  dht.humidity().getSensor(&sensor);
/*  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");*/
  
}

void DHT11_read(){
  Serial.print("DHT11_read ");
  Serial.println(dht_11.active);
  if(dht_11.active){
    sensors_event_t event;  
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
    }
    else {
      mqtt_pub(dht_11.mqtt_temp,String(event.temperature));
      /*Serial.print("Temperature: ");
      Serial.print(event.temperature);
      Serial.println(" *C");*/
    }
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      //Serial.println("Error reading humidity!");
    }
    else {
      mqtt_pub(dht_11.mqtt_hum,String(event.relative_humidity));
      /*Serial.print("Humidity: ");
      Serial.print(event.relative_humidity);
      Serial.println("%");*/
    }
  }
}

void dht11_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_hum")){
     dht_11.mqtt_temp = server.arg("mqtt_temp");
     dht_11.mqtt_hum = server.arg("mqtt_hum");
     dht_11.active = true;
  }
  
  root["mqtt_temp"] = dht_11.mqtt_temp;
  root["mqtt_hum"]  = dht_11.mqtt_hum; 

  if (server.hasArg("mqtt_temp")||server.hasArg("mqtt_hum")){
    File configFile = SPIFFS.open("/dht.json", "w");
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

