#include <iarduino_MultiServo.h>                            // Подключаем библиотеку
iarduino_MultiServo servo;                                    // Объявляем переменную MSS, для работы с библиотекой (объект класса iarduino_MultiServo).

String pca_mqtt_analog="/servo/analog";
String pca_mqtt_ang="/servo/ang";
String pca_mqtt_d="/servo/d";

void pca_init(){
  mqtt_add_out(pca_mqtt_analog+"/#");
  mqtt_add_out(pca_mqtt_ang+"/#");
  mqtt_add_out(pca_mqtt_d+"/#");
}

void pca_save(){
  File configFile = SPIFFS.open("/pca.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["mqtt_analog"]=pca_mqtt_analog;
  root["mqtt_ang"]=pca_mqtt_ang;
  root["mqtt_d"]=pca_mqtt_d;
  
  root.printTo(configFile);
  configFile.close();

  pca_init();
}

void pca_load(){
  DynamicJsonBuffer jsonBuffer;
  
  File configFile = SPIFFS.open("/pca.json", "r");
  if (!configFile) {
    Serial.println("Failed to open PCA config file for reding");
    return;
  }
  
  String line = configFile.readStringUntil('\n');
      //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
   return;
  }

  pca_mqtt_analog = root["mqtt_analog"].as<String>();
  pca_mqtt_ang = root["mqtt_ang"].as<String>();
  pca_mqtt_d = root["mqtt_d"].as<String>();
}

void pca_setup() {  
    pca_load();
  servo.servoSet(SERVO_ALL, SERVO_SG90);
  servo.begin(); 
  pca_init();

}

void pca_mqtt(String mqtt_str08,byte j,String val_s){
  int val = StringToInt(val_s);
  if(pca_mqtt_analog==mqtt_str08){
    servo.analogWrite(j,val);
  };
  
  if(pca_mqtt_ang==mqtt_str08){
    servo.servoWrite(j,val);
  };
  
  if(pca_mqtt_d==mqtt_str08){
    servo.digitalWrite(j,val_s=="0"?LOW:HIGH);
  };
}

void pca9685_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt_ang")){
    pca_mqtt_ang =server.arg("mqtt_ang");
    pca_mqtt_analog=server.arg("mqtt_analog");
    pca_mqtt_d=server.arg("mqtt_d");
  }

  root["mqtt_analog"]=pca_mqtt_analog;
  root["mqtt_ang"]=pca_mqtt_ang;
  root["mqtt_d"]=pca_mqtt_d;

  if (server.hasArg("mqtt_ang")){
    pca_save();
    root["msg"]="Data updated.";
  }

  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}
