void ds2408_config(){
  byte addr[8];
  String old_path;
   
  if(!server.hasArg("code")){
    server.send ( 200, "text/html", "Error" );
    return;
  }
  String option=server.arg("code");

  string_to_addr(option, addr);

  byte t_id=findDS24MQTT(addr);
  Serial.println(option);  
/*ds.reset();
  ds.select(addr);
  ds.write(0x5A);
  ds.write(0xF0);
  ds.write(0x0F);

delay(2000);
  ds.reset();
  ds.select(addr);
  ds.write(0x5A);
  ds.write(0x0F);
  ds.write(0xF0);*/

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt")){
    if(t_id==ds_24_cnt){
      ds_24_cnt++;
      for (byte n=0;n<8;n++){
       ds24_data[t_id].addr[n]=addr[n];
      }
    }
    
    ds24_data[t_id].role=0;
    ds24_data[t_id].invert_=0;
    ds24_data[t_id].start=0;
    if(server.hasArg("role")){
      ds24_data[t_id].role=StringToInt(server.arg("role"));
    }
    if(server.hasArg("invert_")){
      ds24_data[t_id].invert_=StringToInt(server.arg("invert_"));
    }
    
    if(server.hasArg("start")){
      ds24_data[t_id].start=StringToInt(server.arg("start"));
    }
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if(server.argName ( i )=="role[]"){
        ds24_data[t_id].role=ds24_data[t_id].role+StringToInt(server.arg ( i ));
      }
      if(server.argName ( i )=="invert_[]"){
        ds24_data[t_id].invert_=ds24_data[t_id].invert_+StringToInt(server.arg ( i ));
      }
      if(server.argName ( i )=="start[]"){
        ds24_data[t_id].start=ds24_data[t_id].start+StringToInt(server.arg ( i ));
      }
    }
    ds24_data[t_id].mqtt=server.arg("mqtt");
    saveOneWere();

    ds24_data[t_id].last_out=ds24_data[t_id].start;
    byte val = ds24_data[t_id].last_out ^ ds24_data[t_id].invert_;
    DS24_set(ds24_data[t_id].addr,val);

    Serial.println(val);
    root["msg"]="Data updated.";
  }


  if(t_id<ds_24_cnt){
    //Загрузка с базы
    root["code"] = option; 
    root["mqtt"] = ds24_data[t_id].mqtt;

    JsonArray& data = root.createNestedArray("role");
    byte val=1;
    for (int j=0;j<8;j++){
      if(val & ds24_data[t_id].role){
        data.add(val);
      }
      val=val*2;
    }

    JsonArray& data2 = root.createNestedArray("invert_");
    val=1;
    for (int j=0;j<8;j++){
      if(val & ds24_data[t_id].invert_){
        data2.add(val);
      }
      val=val*2;
    }
    
    JsonArray& data3 = root.createNestedArray("start");
    val=1;
    for (int j=0;j<8;j++){
      if(val & ds24_data[t_id].start){
        data3.add(val);
      }
      val=val*2;
    }
  }else{
    //если датчика нет в базе
    root["code"] = option;
    root["mqtt"] = "/home/room1/lamp_test";
    root["role"] = "0";
    root["invert"] = "0";
    root["start"] = "0";
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}
