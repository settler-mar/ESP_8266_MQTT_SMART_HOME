int findDS24MQTT(byte *addr){
//  byte ds_24_cnt=0;
// ds_2406 ds24_data[3];

  int i=0;
  for (;i<ds_24_cnt;i++){
    if (sootv_addr(ds24_data[i].addr,addr)){
      return i;
    }
  }
  return i;
}

void ds2406_config(){
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

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  if (server.hasArg("mqtt")){
    if(t_id==ds_24_cnt){
      ds_24_cnt++;
      for (byte n=0;n<8;n++){
       ds24_data[t_id].addr[n]=addr[n];
      }
    }
    
    ds24_data[t_id].mqtt=server.arg("mqtt");
    ds24_data[t_id].role=StringToInt(server.arg("role"));
    ds24_data[t_id].invert_=StringToInt(server.arg("invert_"));

    saveOneWere();
    root["msg"]="Data updated.";
  }
  
  if(t_id<ds_24_cnt){
    //Загрузка с базы
    root["code"] = option; 
    root["mqtt"] = ds24_data[t_id].mqtt;
    root["role"] = ds24_data[t_id].role;
    root["invert_"] = ds24_data[t_id].invert_;
  }else{
    //если датчика нет в базе
    root["code"] = option;
    root["mqtt"] = "/home/room1/lamp_test";
    root["role"] = "";
    root["invert"] = "";
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );

}

/******read*******
 //https://www.maximintegrated.com/en/app-notes/index.mvp/id/5856
ds.write(0x33); // Read ROM
ds.write(0x55); // Match ROM
ds.write(0xCC); // Skip ROM
ds.write(0x0F); // Search ROM
ds.write(0x4E); // Write Scratchpad
ds.write(0xBE); // Read Scratchpad
ds.write(0x48); // Copy Scratchpad
ds.write(0xB8); // Recall EEprom
ds.write(0x44); // Convert Sensor
ds.write(0x45); // Read Sensor
ds.write(0x01); // Reset Counter
ds.write(0x02); // Read Counter
ds.write(0x03); // Read IO-identification
ds.write(0xF3); // Setup PortA
ds.write(0xF4); // Setup PortB
ds.write(0xF5); // Read PortA
ds.write(0xF6); // Read PortB
ds.write(0xF7); // Write PortA
ds.write(0xF8); // Write PortB

 
 */
