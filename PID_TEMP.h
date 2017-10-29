#include <PID_v1.h>


// http://playground.arduino.cc/Code/PIDLibrary
// http://bigbarrel.ru/arduino_pid/

PID PIDtemp(&pid_in, &pid_out, &pid_z, 2.0, 5.0, 1.0, DIRECT);

void pid_temp_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("pid_z_mqtt")){
    pid_temp_in_cnt=0;
    if(server.hasArg("pid_temp_in_mqtt")){
      pid_temp_in_mqtt[0]=StringToInt(server.arg("pid_temp_in_mqtt"));
      pid_temp_in_cnt=1;
    }
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if(server.argName ( i )=="pid_temp_in_mqtt[]"){
        pid_temp_in_mqtt[pid_temp_in_cnt] = (String)server.arg ( i );
        pid_temp_in_cnt++;
      }
    }
    
    pid_z = StringToFloat(server.arg("pid_z"));
    pid_z_mqtt = server.arg("pid_z_mqtt");
    pid_out_mqtt = server.arg("pid_out_mqtt");
    pid_out_mqtt_z = server.arg("pid_out_mqtt_z");
    pid_temp_in_v = server.arg("pid_temp_in_v");
    pid_out_relay_mqtt = server.arg("pid_out_relay_mqtt");
    pid_out_max = StringToFloat(server.arg("pid_out_max"));
    pid_p = StringToFloat(server.arg("pid_p"));
    pid_i = StringToFloat(server.arg("pid_i"));
    pid_d = StringToFloat(server.arg("pid_d"));
    pid_out_d = StringToFloat(server.arg("pid_out_d"));
    pid_time = StringToInt(server.arg("pid_t"));
    if(pid_time<3)pid_time=3;

    PIDtemp.SetOutputLimits(25, pid_out_max);
    PIDtemp.SetTunings(pid_i, pid_i, pid_d);
  }

  JsonArray& data = root.createNestedArray("pid_temp_in_mqtt");
  for (int j=0;j<pid_temp_in_cnt;j++){
    data.add(pid_temp_in_mqtt[j]);
  }
  
  root["pid_z_mqtt"]=pid_z_mqtt;
  root["pid_z"]=pid_z;
  root["pid_out_mqtt"]=pid_out_mqtt;
  root["pid_temp_in_v"]=pid_temp_in_v;
  root["pid_out_mqtt_z"]=pid_out_mqtt_z;
  root["pid_out_relay_mqtt"]=pid_out_relay_mqtt;
  root["pid_out_max"]=pid_out_max;
  root["pid_p"]=pid_p;
  root["pid_i"]=pid_i;
  root["pid_d"]=pid_d;
  root["pid_t"]=pid_time;
  root["pid_out_d"]=pid_out_d;

  if (server.hasArg("pid_z_mqtt")){
    PID_TEMP_save();   
    root["msg"]="Data updated.";
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

void PID_TEMP_init(){
  for (int j=0;j<10;j++){
    pid_temp_in_mqtt[j]="";
    pid_temp_in[j]=0;
  }
  pid_temp_in_cnt=0;
  pid_z_mqtt="/pid/in";
  pid_z=24.3;
  pid_out_mqtt="/pid/out_val";
  pid_out_mqtt_z="/pid/pid_out_mqtt_z";
  pid_temp_in_v="/pid/pid_temp_in";
  pid_out_relay_mqtt="/pid/relay";
  pid_out_max=90;
  pid_p=8.6;
  pid_i=0.5;
  pid_d=1.1;
  pid_time=10;
  pid_out_d=1.0;
  pid_relay_status=0;
  pid_out_t=25;

  PIDtemp.SetMode(AUTOMATIC);
  
  DynamicJsonBuffer jsonBuffer;
  
  File configFile = SPIFFS.open("/pid.json", "r");
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
    for( const auto& value : root["pid_temp_in_mqtt"].as<JsonArray>() ) {
      pid_temp_in_mqtt[pid_temp_in_cnt]=value.as<String>();
      mqtt_add_out(pid_temp_in_mqtt[pid_temp_in_cnt]);
      pid_temp_in_cnt++;
    }
      
    pid_z = StringToFloat(root["pid_z"].as<String>());
    pid_z_mqtt = root["pid_z_mqtt"].as<String>();
    pid_out_mqtt = root["pid_out_mqtt"].as<String>();
    pid_out_mqtt_z = root["pid_out_mqtt_z"].as<String>();
    pid_temp_in_v = root["pid_temp_in_v"].as<String>();
    pid_out_relay_mqtt = root["pid_out_relay_mqtt"].as<String>();
    pid_out_max = StringToFloat(root["pid_out_max"].as<String>());
    pid_p = StringToFloat(root["pid_p"].as<String>());
    pid_i = StringToFloat(root["pid_i"].as<String>());
    pid_d = StringToFloat(root["pid_d"].as<String>());
    pid_time = StringToInt(root["pid_t"].as<String>());
    if(pid_time<3)pid_time=3;
    pid_out_d = StringToFloat(root["pid_out_d"].as<String>());
  }

  mqtt_add_out(pid_z_mqtt);
  mqtt_add_out(pid_out_mqtt);

  PIDtemp.SetOutputLimits(20, pid_out_max);
  PIDtemp.SetTunings(pid_i, pid_i, pid_d);
}

void PID_analiz(int t_sec){
  byte temp_period=(t_sec % (pid_time * T_PERIOD));
  if(temp_period==T_PERIOD){
    //Serial.print("t_sec= ");
    //Serial.println(t_sec);
    byte k;
    byte i;
    k=0;
    pid_in=0;
    for(i=0;i<pid_temp_in_cnt;i++  ) {
      if(pid_temp_in[i]!=0){
        k++;
        pid_in=pid_in+pid_temp_in[i];
      }
    }
    if(k==0)return;
    pid_in=pid_in/k;
    PIDtemp.Compute();
    /*Serial.print("pid_in= ");
    Serial.println(pid_in);
    Serial.print("pid_z= ");
    Serial.println(pid_z);
    Serial.print("pid_out= ");
    Serial.println(pid_out);
    Serial.print("pid_relay_status= ");
    Serial.println(pid_relay_status);
    Serial.println();*/
    String t;
    t=pid_out;
    mqtt_pub(pid_out_mqtt_z,t);
    t=pid_in;
    mqtt_pub(pid_temp_in_v,t);
    t=pid_z;
    mqtt_pub(pid_z_mqtt,t);
        
    //сильное охлаждение
    if(pid_out_t<pid_out-pid_out_d && pid_relay_status==0){
      pid_relay_status=255;
      mqtt_pub(pid_out_relay_mqtt,"255");
    }

    //перегрев
    if(pid_out_t>pid_out+pid_out_d && pid_relay_status!=0){
      pid_relay_status=0;
      mqtt_pub(pid_out_relay_mqtt,"0");
    }
  }
}
