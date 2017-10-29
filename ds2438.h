#define DS2438_CHA 0
#define DS2438_CHB 1

#define DS2438_MODE_CHA 0x01
#define DS2438_MODE_CHB 0x02
#define DS2438_MODE_TEMPERATURE 0x04

int findDS2438MQTT(byte *addr){
  int i=0;
  for (;i<ds_2438_cnt;i++){
    if (sootv_addr(ds2438_data[i].addr,addr)){
      return i;
    }
  }
  return i;
}

void volt_read(int t_sec){
  byte data[12];
  for (int i=0;i<ds_2438_cnt;i++){
    byte temp_period=(t_sec % (ds2438_data[i].intrval*T_PERIOD));
    if(temp_period==0){ //set config
      ds.reset();
      ds.select(ds2438_data[i].addr);   //обращаюсь ко всем
      ds.write(0x4E);  //Issue WRITE SCRATCHPAD command
      ds.write(0x00);  //PAGE 0
      ds.write(0x07);  //SETUP FOR VAD INPUT
    }
    if(temp_period==T_PERIOD){//преобразование
      ds.reset();
      ds.select(ds2438_data[i].addr);   //обращаюсь к узлу
      ds.write(0xb4, 1);   // start conversion, with parasite power on at the end
    }
    if(temp_period==T_PERIOD*2){//RECALL MEMORY
      ds.reset();
      ds.select(ds2438_data[i].addr);
      ds.write(0xB8);  //RECALL MEMORY
      ds.write(0x00);
      //delay(500);
    }
    if(temp_period==T_PERIOD*3-1){  //read
      ds.reset();
      ds.select(ds2438_data[i].addr);
      ds.write(0xBE);  //READ SCRATCHPAD
      ds.write(0x00);
      for (byte j = 0; j < 9; j++) { // we need 9 bytes, last one for CRC
        data[j] = ds.read();
        //Serial.print(data[j], HEX);
        //Serial.print(" ");
      }
      
      int16_t temp;
      //Serial.println(" ");
    
      temp = (int16_t) data[4];
      temp <<= 8;
      temp |= data[3];
      float voltage;
      voltage = (float) temp;
      voltage *= 0.01;
      //Serial.println(voltage);  
      mqtt_pub(ds2438_data[i].mqtt_v,String(voltage));
      
      temp = (int16_t) data[2];
      temp <<= 8;
      temp |= data[1];
      float temperature;
      temperature = (float) temp;
      temperature *= 0.00390625;
      //Serial.println(temperature);
      mqtt_pub(ds2438_data[i].mqtt_t,String(temperature));
    }
  }      
}

void calculate_volt(){
  ds.reset();
  ds.write(0xCC, 1);   //обращаюсь ко всем
  ds.write(0xb4, 1);   // start conversion, with parasite power on at the end

}

void ds2438_config(){
  byte addr[8];  
  if(!server.hasArg("code")){
    server.send ( 200, "text/html", "Error" );
    return;
  }
  String option=server.arg("code");
  string_to_addr(option, addr);
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  byte t_id=findDS2438MQTT(addr);

/*  byte i;
// byte present = 0;
byte data[12];
//configure chip to read VAD and not VDD
ds.reset();
ds.select(addr);
ds.write(0x4E);  //Issue WRITE SCRATCHPAD command
ds.write(0x00);  //PAGE 0
ds.write(0x07);  //SETUP FOR VAD INPUT
//delay(500);

calculate_volt();

ds.reset();
ds.select(addr);
ds.write(0xB8);  //RECALL MEMORY
ds.write(0x00);
//delay(500);

Serial.println(" ");
ds.reset();
ds.select(addr);
ds.write(0xBE);  //READ SCRATCHPAD
ds.write(0x00);
for ( i = 0; i < 9; i++) { // we need 9 bytes, last one for CRC
  data[i] = ds.read();
  Serial.print(data[i], HEX);
  Serial.print(" ");
}
int16_t temp;
Serial.println(" ");

    temp = (int16_t) data[4];
    temp <<= 8;
    temp |= data[3];
    float voltage;
    voltage = (float) temp;
    voltage *= 0.01;
  Serial.println(voltage);  

     temp = (int16_t) data[2];
    temp <<= 8;
    temp |= data[1];
    float temperature;
    temperature = (float) temp;
    temperature *= 0.00390625;
    Serial.println(temperature);  */

    
  if (server.hasArg("mqtt_v")){
    if(t_id==ds_2438_cnt){
      ds_2438_cnt++;
      for (byte n=0;n<8;n++){
       ds2438_data[t_id].addr[n]=addr[n];
      }
    }
    ds2438_data[t_id].mqtt_v=server.arg("mqtt_v");
    ds2438_data[t_id].mqtt_t=server.arg("mqtt_t");
    ds2438_data[t_id].intrval=StringToInt(server.arg("intrval"));
    if(ds2438_data[t_id].intrval<3)ds2438_data[t_id].intrval=3;
    root["msg"]="Data updated.";

    saveOneWere();
    init_2438(addr);
  }

  if(t_id<ds_2438_cnt){
    //Загрузка с базы
    root["code"] = option; 
    root["mqtt_v"] = ds2438_data[t_id].mqtt_v;
    root["mqtt_t"] = ds2438_data[t_id].mqtt_t;
    root["intrval"] = ds2438_data[t_id].intrval;
  }else{
    //если датчика нет в базе
    root["code"] = option;
    root["mqtt_v"] = "/home/room1/analog_test";
    root["mqtt_t"] = "/home/room1/temperature_test";
    root["intrval"]=3;
  }
  
  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

