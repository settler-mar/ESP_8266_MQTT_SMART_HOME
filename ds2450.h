int findDS2450MQTT(byte *addr){
  int i=0;
  for (;i<ds_2450_cnt;i++){
    if (sootv_addr(ds2450_data[i].addr,addr)){
      return i;
    }
  }
  return i;
}

void calculate_2450_volt(byte *addr){
  ds.reset();
  ds.select(addr);
  ds.write(0x3c, 1);//DS2450_BEGIN_VOLTAGE_CONVERSION
  ds.write(0x0f, 1);//DS2450_REGISTER_4CHANNEL
  ds.write(0xaa, 1);//DS2450_READOUT_CONTROL
  ds.read(); // crc
  ds.read();
}

void volt_2450_read(int t_sec){
  byte data[12];
  for (int i=0;i<ds_2450_cnt;i++){
    byte temp_period=(t_sec % (ds2450_data[i].intrval*T_PERIOD));
    if(temp_period==0){ //set config
      init_2450(ds2450_data[i].addr);
    }
    if(temp_period==T_PERIOD){//преобразование
      calculate_2450_volt(ds2450_data[i].addr);
    }
        
    if(temp_period==T_PERIOD*2-1){  //read
     // Serial.println("read ds2450");
      ds.reset();
      ds.select(ds2450_data[i].addr);
      ds.write(0xaa);  //DS2450_MEMORY_READ_COMMAND
      ds.write(0x00); //DS2450_AD_CHANNELS_ADDR_LO
      ds.write(0x00);//DS2450_AD_CHANNELS_ADDR_HI
      for (byte j = 0; j < 10; j++) { // we need 9 bytes, last one for CRC
        data[j] = ds.read();
      }
      float volt;

      volt=(float)data[1] / 50.0;
      mqtt_pub(ds2450_data[i].mqtt+"/0",String(volt));
      
      volt=(float)data[3] / 50.0;
      mqtt_pub(ds2450_data[i].mqtt+"/1",String(volt));
      
      volt=(float)data[5] / 50.0;
      mqtt_pub(ds2450_data[i].mqtt+"/2",String(volt));
      
      volt=(float)data[7] / 50.0;
      mqtt_pub(ds2450_data[i].mqtt+"/3",String(volt));
    }
  }      
}
/*
   Serial.println("init");
      ds.reset();
      ds.write(0xCC, 1);
      ds.write(0x55); 
      ds.write(0x1c); 
      ds.write(0x00); 
      ds.write(0x40);
      ds.read();
      ds.read();
      ds.read();
delay(1000);
Serial.println("config");
      ds.reset();
      ds.write(0xCC, 1);
      ds.write(0x55); 
      ds.write(0x08); 
      ds.write(0x00); 
     for (int i = 0; i < 4; i++) {
       ds.write(0x08, 0);
        ds.read(); // crc
        ds.read();
        ds.read(); // verify data
        ds.write(0x01, 0);
        ds.read(); // crc
        ds.read();
        ds.read(); // verify data
    }
    delay(1000);
Serial.println("convert");
            ds.reset();
            ds.write(0xCC, 1);
      ds.write(0x3c, 1);
      ds.write(0x0f, 1);
      ds.write(0xaa, 1);
        ds.read(); // crc
        ds.read();
      
      delay(1000);
      Serial.println("read");
      ds.reset();
      ds.write(0xCC, 1);
      ds.write(0xaa);  
      ds.write(0x00); 
      ds.write(0x00);
      for (byte j = 0; j < 10; j++) { // we need 9 bytes, last one for CRC
        data[j] = ds.read();
        Serial.print(data[j], HEX);
        Serial.print(" ");
      }
      Serial.println("");
      float volt;

      volt=(float)data[1] / 50.0;
      Serial.print("V1 = ");
      Serial.println(volt);
      
      volt=(float)data[3] / 50.0;
      Serial.print("V2 = ");
      Serial.println(volt);
      
      volt=(float)data[5] / 50.0;
      Serial.print("V3 = ");
      Serial.println(volt);
      
      volt=(float)data[7] / 50.0;
      Serial.print("V4 = ");
      Serial.println(volt);
      delay(1000);
 */
void ds2450_config(){
  byte addr[8];  
  if(!server.hasArg("code")){
    server.send ( 200, "text/html", "Error" );
    return;
  }
  String option=server.arg("code");
  string_to_addr(option, addr);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  byte t_id=findDS2450MQTT(addr);

    
  if (server.hasArg("mqtt")){
    if(t_id==ds_2450_cnt){
      ds_2450_cnt++;
      for (byte n=0;n<8;n++){
       ds2450_data[t_id].addr[n]=addr[n];
      }
    }
    ds2450_data[t_id].mqtt=server.arg("mqtt");
    ds2450_data[t_id].intrval=StringToInt(server.arg("intrval"));
    if(ds2450_data[t_id].intrval<3)ds2450_data[t_id].intrval=3;
    root["msg"]="Data updated.";

    saveOneWere();
    init_2450(addr);
  }

  if(t_id<ds_2450_cnt){
    //Загрузка с базы
    root["code"] = option; 
    root["mqtt"] = ds2450_data[t_id].mqtt;
    root["intrval"] = ds2450_data[t_id].intrval;
  }else{
    //если датчика нет в базе
    root["code"] = option;
    root["mqtt"] = "/home/room1/analog_test";
    root["intrval"]=3;
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

