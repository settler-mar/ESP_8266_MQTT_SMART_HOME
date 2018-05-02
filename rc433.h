/* 
  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>
RCSwitch rc433 = RCSwitch();

int rc433_last;
unsigned long rc433_time;
String rc433_mqtt;

void rc433_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt")){
     rc433_mqtt = server.arg("mqtt");;
  }
  
  root["mqtt"]=rc433_mqtt;


  if (server.hasArg("mqtt")){
    File configFile = SPIFFS.open("/rc433.json", "w");
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

void rc433_init(){
  rc433.enableReceive(RC433_PORT);

  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/rc433.json", "r");
  if (!configFile) {
    Serial.println("Failed to open rc433 config file for reding");
    rc433_mqtt = "/rc433";
    return;
  }
  
  String line = configFile.readStringUntil('\n');
    //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
  }else{
     rc433_mqtt = root["mqtt"].as<String>();
  }
}

void rc433_do(){
  if (rc433.available()) {
    
    int value = rc433.getReceivedValue();
    
    if (value == 0 || (rc433_last==value && rc433_time>0)) {

    } else {
      rc433_last = value;
      rc433_time=millis();

      mqtt_pub(rc433_mqtt,String(value));
      /*Serial.print("Received ");
      Serial.print( rc433.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( rc433.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( rc433.getReceivedProtocol() );*/
    }

    rc433.resetAvailable();
  }

  if(rc433_time>0){
    if(millis()-rc433_time>1000){
      rc433_time=0;
    }
  }
}

