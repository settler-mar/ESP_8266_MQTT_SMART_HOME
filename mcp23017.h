#include "Adafruit_MCP23017.h"

Adafruit_MCP23017 mcp;

word mcp_role=255;
word mcp_invert=0;
word mcp_mode=0;
word mcp_timer[16];
//word mcp_status=0;
String mcp_mqtt_="/test/mcp";

word mcp_last_satate;
#ifdef UN_drebizg
  word mcp_last_satate_dreb[UN_drebizg];
  byte mcp_last_satate_dreb_cnt=0;
#endif

void mcp_init(){
  if(mcp_role!=65535){
    mqtt_add_out(mcp_mqtt_+"/#");
  }

  word val=1;
  for (int j=0;j<16;j++){
    if(val & mcp_role){
      mcp.pinMode(j, INPUT);
      mcp.pullUp(j, LOW);  // turn on a 100K pullup internally
      mcp.setupInterruptPin(j,CHANGE);
    }else{
      mcp.pinMode(j, OUTPUT);
    }
    val=val*2;
  }
  
  /*for(i=0;i<16;i++){
    mcp.pinMode(i, OUTPUT);
  }
  //mcp.pullUp(0, HIGH);  // turn on a 100K pullup internally*/
}

void mcp_save(){
  File configFile = SPIFFS.open("/mcp.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["mqtt"]=mcp_mqtt_;
  root["invert"]=mcp_invert;
  root["role"]=mcp_role;
  root["mode"]=mcp_mode;

  root.printTo(configFile);
  configFile.close();

  mcp_init();
}

void mcp_load(){
  DynamicJsonBuffer jsonBuffer;
  
  File configFile = SPIFFS.open("/mcp.json", "r");
  if (!configFile) {
    Serial.println("Failed to open MCP config file for reding");
    return;
  }
  
  String line = configFile.readStringUntil('\n');
      //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
   return;
  }

  mcp_mqtt_ = root["mqtt"].as<String>();
  mcp_role = StringToWord(root["role"].as<String>());
  mcp_invert = StringToWord(root["invert"].as<String>());
  mcp_mode = StringToWord(root["mode"].as<String>());
}

void mcp_setup() {  
  byte i;
  mcp.begin(MCP);      // use default address 0
  mcp.setupInterrupts(true,false,LOW);
  
  mcp_load();
  mcp_init();

  mcp_last_satate=mcp.readGPIOAB();
}

void mcp_loop(){
  uint8_t buf=mcp.readGPIOAB();
  uint8_t t_buf;
  word val=1;
  buf= mcp_role & buf;

  if(mcp_mode>0){ 
    word t=mcp_mode & (65535^mcp_role);
    for (int j=0;j<16;j++){
      if(val & t){
        mcp_timer[j]++;
        if(mcp_timer[j]>=pulse_perion){
          mcp_timer[j]=0;
          mcp.digitalWrite(j, LOW);
        }
      }
      val=val*2;
    }
  }
  
  #ifdef UN_drebizg
    t_buf=buf;
    uint8_t t_buf_n=buf;
    for (int j=0;j<UN_drebizg;j++){
      t_buf=t_buf & mcp_last_satate_dreb[j];
      t_buf_n=t_buf_n | mcp_last_satate_dreb[j];
    }
    mcp_last_satate_dreb[mcp_last_satate_dreb_cnt]=buf;
    mcp_last_satate_dreb_cnt++;
    if(mcp_last_satate_dreb_cnt>=UN_drebizg)mcp_last_satate_dreb_cnt=0;
    
    buf=t_buf^t_buf_n;
    t_buf=255^buf;
    buf = (mcp_last_satate & buf) | (t_buf & t_buf_n);
  #endif

  t_buf=buf ^ mcp_last_satate;
  if(t_buf==0)return;
  mcp_last_satate=buf;
  //uint8_t buf = t_buf^mcp_invert;
  buf = buf ^ mcp_invert;

  /*Serial.print("t_buf: ");
  Serial.print(t_buf);
  Serial.print(" ");
  Serial.println(buf);/**/
  
  val=1;
  byte out;
  for (int j=0;j<16;j++){
    if(val & t_buf){
      /*Serial.print("val: ");
      Serial.print(val);
      Serial.print(" ");
      Serial.print(val & t_buf);
      Serial.print(" ");
      Serial.println(j);/**/
      out = (val & mcp_last_satate)?255:0;
      mqtt_pub(mcp_mqtt_+"/"+String(j), String(out));
    }
    val=val*2;
  }
}

void mcp_mqtt(String mqtt_str08,byte j,String val_s){
  if(mcp_mqtt_!=mqtt_str08)return;
  word role = mcp_role;
  if(j>0){
    role=role>>j;
  }
  if(role&1)return;
  byte val=(val_s=="0"?0:1);
  word inv = mcp_invert;
  if(j>0){
    inv=inv>>j;
  };
  inv = inv & 1;
  val=val^inv;

  //Serial.println(val);
  //Serial.print(" ");
  
  val=((val&1)==0?LOW:HIGH);
  //Serial.println(val);
  //Serial.println(" ");
  mcp.digitalWrite(j, val);

  if(val){
    mcp_timer[j]=0;
    //mcp_status=mcp_status|(1<<j);
  }else{
    //mcp_status=(mcp_status|(1<<j))^(1<<j);
  }
}

void mcp23017_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt")){
    mcp_mqtt_ =server.arg("mqtt");

    mcp_role=0;
    mcp_invert=0;
    mcp_mode=0;
    if(server.hasArg("role")){
      mcp_role=StringToInt(server.arg("role"));
    }
    if(server.hasArg("mode")){
      mcp_mode=StringToInt(server.arg("mode"));
    }
    if(server.hasArg("invert_")){
      mcp_invert=StringToInt(server.arg("invert_"));
    }

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if(server.argName ( i )=="role[]"){
        mcp_role=mcp_role+StringToInt(server.arg ( i ));
      }
      if(server.argName ( i )=="invert_[]"){
        mcp_invert=mcp_invert+StringToInt(server.arg ( i ));
      }
      if(server.argName ( i )=="mode[]"){
        mcp_mode=mcp_mode+StringToInt(server.arg ( i ));
      }
    }
  }
  
  JsonArray& data = root.createNestedArray("role");
  word val=1;
  for (int j=0;j<16;j++){
    if(val & mcp_role){
      data.add(val);
    }
    val=val*2;
  }
  
  JsonArray& data2 = root.createNestedArray("invert_");
  val=1;
  for (int j=0;j<16;j++){
    if(val & mcp_invert){
      data2.add(val);
    }
    val=val*2;
  }
  root["mqtt"]=mcp_mqtt_;

  JsonArray& data3 = root.createNestedArray("mode");
  val=1;
  for (int j=0;j<16;j++){
    if(val & mcp_mode){
      data3.add(val);
    }
    val=val*2;
  }
  root["mqtt"]=mcp_mqtt_;

  if (server.hasArg("mqtt")){
    mcp_save();
    root["msg"]="Data updated.";
  }

  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

