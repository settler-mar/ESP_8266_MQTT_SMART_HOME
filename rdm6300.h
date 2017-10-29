#ifdef RDM6300_APARAT_UART
  
#else
  SoftwareSerial RDM6300_SERIAL(RDM6300_SOFT_UART_RX,RDM6300_SOFT_UART_TX);
#endif

String RDM6300_buff;
String RDM6300_prev;
int RDM6300_cnt;
String rdm6300_mqtt;

void RDM6300_init(){
  #ifdef RDM6300_APARAT_UART
    Serial.begin(9600);
  #else
    RDM6300_SERIAL.begin(9600);
  #endif
  RDM6300_buff="";
  RDM6300_prev="";
  RDM6300_cnt=0;

  DynamicJsonBuffer jsonBuffer;

  rdm6300_mqtt="/gardeb/rfid";
  
  File configFile = SPIFFS.open("/rdm6300.json", "r");
  if (!configFile) {
    Serial.println("Failed to open RDM6300 config file for reding");
    return;
  }
  
  String line = configFile.readStringUntil('\n');
    //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
  }else{
     rdm6300_mqtt = root["mqtt"].as<String>();
  }
}

void RDM6300_render(){
  byte buf;
  #ifdef RDM6300_APARAT_UART
    while (Serial.available() > 0) {    
      buf=Serial.read();
  #else
    while (RDM6300_SERIAL.available() > 0) {
      buf=RDM6300_SERIAL.read();
  #endif
        
      if(buf==0x03){
        if(RDM6300_prev!=RDM6300_buff){
          //Serial.println(RDM6300_buff);
          mqtt_pub(rdm6300_mqtt,RDM6300_buff);
          RDM6300_prev=RDM6300_buff;
        }
      }else if(buf==0x02){
        RDM6300_buff="";
        RDM6300_cnt=0;
      }else{
        RDM6300_buff+=char(buf);
      }
    }

  RDM6300_cnt++;
  if(RDM6300_cnt>3){
    RDM6300_prev="";
  }
}


void rdm6300_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt")){
     rdm6300_mqtt = server.arg("mqtt");
  }
  
  root["mqtt"] = rdm6300_mqtt;


  if (server.hasArg("mqtt")){
    File configFile = SPIFFS.open("/rdm6300.json", "w");
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

