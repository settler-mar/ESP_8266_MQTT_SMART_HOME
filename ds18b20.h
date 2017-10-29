float getTemp(byte* addr){
  byte type_s;
  byte data[12];
  float celsius;

    /*ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);   // start conversion, with parasite power on at the end

    delay(1000);*/
  switch (addr[0]) {
    case 0x10:
      //Chip = DS18S20
      type_s = 1;
      break;
    case 0x28:
      //Chip = DS18B20
      type_s = 0;
      break;
    case 0x22:
      //Chip = DS1822;
      type_s = 0;
      break;
    default:
      return 0;
  } 
  
  //calculate_temp();
  //delay(1000);     // maybe 750ms is enough, maybe not

  ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  
  for (byte i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  
  celsius = (float)raw / 16.0;
/*  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");*/
  
  return celsius;
}

void setDS1820_params(byte *addr){
  //задаем точность
  ds.reset();
  ds.select(addr);
  ds.write(0x4E, 1);
  
  ds.write(0, 1);
  ds.write(0, 1);
  //ds.write(0x1F, 1); //9 bit - 93ms - 0.5*C
  ds.write(0x3F, 1); //10 bit - 187 ms - 0.25*C
  //ds.write(0x5F, 1); //11 bit - 375 ms - 0.125*C
  //ds.write(0x7F, 1); //12 bit - 750 ms - 0.0625*C


  ds.reset(); // reset 1-Wire
  ds.select(addr); // select DS18B20
  ds.write(0x48); // copy scratchpad to EEPROM
  delay(15); // wait for end of write

}

void calculate_temp(byte i){

#ifdef DS_1820_personal_convert
  /*Serial.println();
  Serial.print("Start convert ");
  Serial.println(i);*/

  /*if(i==0){
    ds.reset();
    ds.write(0xCC);   //обращаюсь ко всем
    ds.write(0x44);   // start conversion, with parasite power on at the end
  }*/
  
  if(i<ds_temp_cnt){
    /*Serial.print("ROM =");
    for(byte j = 0; j < 8; j++) {
      Serial.write(' ');
      Serial.print(ds_temp[i].addr[j], HEX);
    }
    Serial.println();  */

    setDS1820_params(ds_temp[i].addr);
  
    ds.reset();
    ds.select(ds_temp[i].addr);
    ds.write(0x44);   // start conversion, with parasite power on at the end
    delay(200);
  }
#else
  if(i==0){
    ds.reset();
    ds.write(0xCC);   //обращаюсь ко всем
    ds.write(0x44);   // start conversion, with parasite power on at the end
    return;
  }
#endif
}

int findDStempMQTT(byte *addr){
  int i=0;
  for (;i<ds_temp_cnt;i++){
    if (sootv_addr(ds_temp[i].addr,addr)){
      return i;
    }
  }
  return i;
}

void saveDStempMQTT(int i,byte *addr, String mqtt){
  if(i==ds_temp_cnt){
    for (byte n=0;n<8;n++){
     ds_temp[i].addr[n]=addr[n];
    }
    ds_temp_cnt++;
  }
  ds_temp[i].mqtt=mqtt;
  saveOneWere();
  return ;
}


void mqtt_temp(byte i){
  String Cel;
  float celsiy;
  //for (int i=0;i<ds_temp_cnt;i++){
  if(i==0){
    return;
  }
  i=i-1;
  if(i<ds_temp_cnt){
    celsiy=getTemp(ds_temp[i].addr);
    Cel=celsiy;
    if(celsiy==85 || celsiy > 127){
      Serial.print(ds_temp[i].mqtt);
      Serial.print('-');
      Serial.print(celsiy);
      Serial.println("ERR not send");
    }else{
      mqtt_pub(ds_temp[i].mqtt.c_str(), Cel.c_str());
    }
    //delay(20);
  }
}

void ds18b20_config(){
  byte addr[8];
  float celsiy;
  String old_path;
   
  if(!server.hasArg("code")){
    server.send ( 200, "text/html", "Error" );
    return;
  }
  String option=server.arg("code");

  string_to_addr(option, addr);

  byte t_id=findDStempMQTT(addr);
  
  celsiy=getTemp(addr);
  String Cel="";
  Cel+=celsiy;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  if (server.hasArg("mqtt")){
    String mqtt = server.arg("mqtt");
    Serial.println(mqtt);
    saveDStempMQTT(t_id,addr, mqtt);
    root["msg"]="Data updated.";
  }

  if(t_id<ds_temp_cnt){
    old_path=ds_temp[t_id].mqtt;
  }else{
    old_path="/home/room1/temp_test";
  }

  root["code"] = option;
  root["mqtt"] = old_path;
  root["temper"] = Cel;
    
//option.c_str(),old_path.c_str(),Cel.c_str()
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );

}
