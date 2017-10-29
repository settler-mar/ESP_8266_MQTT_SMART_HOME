// sudo /etc/init.d/mosquitto start
// https://github.com/256dpi/arduino-mqtt/blob/master/examples/AdafruitHuzzahESP8266/AdafruitHuzzahESP8266.ino

int subsribe;

void mqtt_pub(String mqtt_addr,String data){
  if(mqtt_addr.length()<5)return;
  Serial.print(mqtt_addr);
  Serial.print(" - ");
  Serial.println(data);
  if (mqtt.connected()) {
    mqtt.publish(mqtt_addr,data);
  }
}
void DS24_set(byte addr[8],byte val){
  ds.reset();
  if(addr[0]==0x12){
    ds.select(addr);
    ds.write(0xf5);
    ds.write(0x04);
    ds.write(0x00);
    ds.read();
    ds.write(val);
  }
//  Serial.println(addr[0]);
//  Serial.println(val);

  if(addr[0]==0x85 && (val & 4)){
    val=val+2;
  }
  if(addr[0]==0x29||addr[0]==0x85){
    ds.reset();
    ds.select(addr);
    ds.write(0x5A);
    ds.write(val);
    ds.write(~val);
  }  
}

void updateMQTT(String mqtt_str,String val_s){
  int i=0;
  byte val;
  val=StringToInt(val_s);
  String mqtt_str08 = mqtt_str.substring(0, mqtt_str.length()-2);
  byte j= StringToInt( mqtt_str.substring(mqtt_str.length()-1));
  
 
  for (;i<ds_24_cnt;i++){
    if (ds24_data[i].addr[0]==0x12 && ds24_data[i].role==2 && ds24_data[i].mqtt==mqtt_str){
      if(val^ds24_data[i].invert_){
        DS24_set(ds24_data[i].addr,0xFF);//значение выкл
      }else{
        DS24_set(ds24_data[i].addr,0x00); //значение вкл    
      }
    }
    if (ds24_data[i].addr[0]!=0x12 && ds24_data[i].mqtt==mqtt_str){
      ds24_data[i].last_out=(val^ds24_data[i].invert_)&(~ds24_data[i].role);
      byte t_val=ds24_data[i].last_out|(ds24_data[ds_24_cnt].role ^ ds24_data[ds_24_cnt].start);
      
      DS24_set(ds24_data[i].addr,t_val);
    }
    
    if (ds24_data[i].addr[0]!=0x12 && ds24_data[i].mqtt==mqtt_str08){
      byte v=0;
      byte n= 1 << j;
      if((~ds24_data[i].role)&n){
        if(val){
          ds24_data[i].last_out=(ds24_data[i].last_out|n); //зажечь
        }else{
          n=~n;
          ds24_data[i].last_out=(ds24_data[i].last_out&n);//погасить
        }
        val=ds24_data[i].last_out^ds24_data[i].invert_;
        val=val&(~ds24_data[i].role);
        val=val| (ds24_data[ds_24_cnt].role ^ ds24_data[ds_24_cnt].start);
        DS24_set(ds24_data[i].addr,val);
      }
      /*if(val^ds24_data[i].invert_){
        DS24_set(ds24_data[i].addr,0xFF);//значение выкл
      }else{
        DS24_set(ds24_data[i].addr,0x00); //значение вкл    
      }*/
    }
    
  }

  /*Проверяем данные для PID регулятора*/
  #ifdef PID_TEMP
    for(i=0;i<pid_temp_in_cnt;i++  ) {
      if(pid_temp_in_mqtt[i]==mqtt_str){
        pid_temp_in[i]=StringToFloat(val_s);
      }
    }

    if(pid_z_mqtt==mqtt_str){
      pid_z=StringToFloat(val_s);
      PID_TEMP_save();
    }

    if(pid_out_mqtt==mqtt_str){
      pid_out_t=StringToFloat(val_s);
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
  #endif
  
  return;
}

//обрвботка всех выключателей
void mqtt_24(){
  int i=0;
  ds.reset();
  for (;i<ds_24_cnt;i++){
    if (
        (ds24_data[i].addr[0]==0x12 && ds24_data[i].role==1)||
        (ds24_data[i].addr[0]==0x29 && ds24_data[i].role) ||
        (ds24_data[i].addr[0]==0x85 && ds24_data[i].role)
      ){
      ds.select(ds24_data[i].addr);
      ds.write(0xF5);
      ds.write(0x44);//ALR=0;IM=1;TOG=0;IC=0;CHS1=0;CHS0=1;CRC1=0;CRC0=0
      ds.write(0xFF);
      ds.write(0x44);//ALR=0;IM=1;TOG=0;IC=0;CHS1=0;CHS0=1;CRC1=0;CRC0=0
      ds.write(0xFF);
      byte buf=ds.read();
      ds.reset();

      if(ds24_data[i].addr[0]==0x29 || ds24_data[i].addr[0]==0x85){
        buf=buf&ds24_data[i].role;
      }
           
      #ifdef UN_drebizg
        byte t_buf=buf;
        byte t_buf_n=buf;
        for(int j=0;j<UN_drebizg-1;j++){
          t_buf=t_buf & ds24_data[i].last_satate_dreb[j];
          t_buf_n=t_buf_n | ds24_data[i].last_satate_dreb[j];
          ds24_data[i].last_satate_dreb[j]=ds24_data[i].last_satate_dreb[j+1];
        }
        ds24_data[i].last_satate_dreb[UN_drebizg-1]=buf;
           
        buf=t_buf^t_buf_n;
        t_buf=255^buf;
        buf=(ds24_data[i].last_satate & buf) | (t_buf & t_buf_n);

      
      #endif
      
      if(ds24_data[i].last_satate!=buf){
        //Serial.println(buf);
        
        if(ds24_data[i].addr[0]==0x12){
          if(ds24_data[i].invert_==1){
            buf=!buf;
          }
          if(buf){
            mqtt_pub(ds24_data[i].mqtt,"255");//значение выкл
          }else{
            mqtt_pub(ds24_data[i].mqtt,"0"); //значение вкл    
          } ;
        };
        if(ds24_data[i].addr[0]==0x29 || ds24_data[i].addr[0]==0x85){
          byte j = ds24_data[i].last_satate^buf;
          byte inv=buf^ds24_data[i].invert_;
          if(j){
            byte q=0;
            for (;q<8;q++){
              byte mask = 1<<q;
              if(mask&j){
                byte val=mask&inv;
                if(val){
                  mqtt_pub(ds24_data[i].mqtt+"/"+String(q),"255");
                }else{
                  mqtt_pub(ds24_data[i].mqtt+"/"+String(q),"0");
                }
              }
            }
          }
        }

        ds24_data[i].last_satate=buf;
      }
    }
  }
  return;  
};

void messageReceived(String &topic, String &payload) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();

  updateMQTT(topic,payload);
}

void mqtt_subscribe(){
  Serial.println();
  Serial.println("Mqtt subscribe:");
  for (int i = 0; i < mqtt_count; i++){
    Serial.print(i);
    Serial.print(") ");
    Serial.println(mqtt_output[i]);
    
    if (mqtt_output[i].length()>5 && mqtt.connected()) {
      /*if(initController){
        mqtt_pub(mqtt_output[i],"0");
      }*/
      mqtt.subscribe(mqtt_output[i]);
    }
  }
  Serial.println("MQTT subscribe finish");
  subsribe=1;
}

void mqtt_init(){
  subsribe=0;

 
  File configFile = SPIFFS.open("/config.json", "r");
  size_t size = configFile.size();

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& configJSON = jsonBuffer.parseObject(buf.get());

  configJSON.success();
  const char* mqtt_server=configJSON["mqtt_server"];

  
  //mqtt
  //String n1=String(ESP.getChipId());
  String n1=String(myIP);
  const char *n=StringToChar(n1);
  Serial.println();
  Serial.print("MQTT cient name: ");
  Serial.println(n1);
  Serial.print("MQTT init to server: ");
  Serial.print(config.mqtt_server);
  mqtt.begin(mqtt_server, espClient);
  mqtt.onMessage(messageReceived);
  mqtt.connect(n);

  /*while (!mqtt.connect(n)) {
  //while (!mqtt.connect("My_dev")){
    Serial.print(".");
    //delay(1000);
  }*/
  //delay(1000);
  //mqtt_subscribe();
  Serial.println();
}

void mqtt_add_out(String txt){
  if(txt.length()<5)return;
  for (int i = 0; i < mqtt_count; i++){
    if(mqtt_output[i]==txt){
      return;
    }
  }
  mqtt_output[mqtt_count]=txt;
  mqtt_count++;
  return;
}
