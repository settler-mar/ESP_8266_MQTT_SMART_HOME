void init_2438(byte *addr){
  ds.reset();
  ds.select(addr);
  ds.write(0x4E);  //Issue WRITE SCRATCHPAD command
  ds.write(0x00);  //PAGE 0
  ds.write(0x07);  //SETUP FOR VAD INPUT
}

void init_2450(byte *addr){
  ds.reset();
  ds.select(addr);
  ds.write(0x55); //DS2450_WRITE_COMMAND
  ds.write(0x1c); //DS2450_VCC_CONTROL_BYTE_ADDR_LO
  ds.write(0x00); //DS2450_VCC_CONTROL_BYTE_ADDR_HI
  ds.write(0x40);//DS2450_VCC_POWERED
  ds.read();
  ds.read();
  ds.read();

  ds.reset();
  ds.select(addr);
  ds.write(0x55); //DS2450_WRITE_COMMAND
  ds.write(0x08); //DS2450_CONTROL_STATUS_DATA_ADDR_LO
  ds.write(0x00); //DS2450_CONTROL_STATUS_DATA_ADDR_HI
  for (int i = 0; i < 4; i++) {
    ds.write(0x08, 0);//DS2450_8_BIT_RESOLUTION
    ds.read(); // crc
    ds.read();
    ds.read(); // verify data
    ds.write(0x01, 0);//DS2450_POR_OFF_NO_ALARMS_5V_RANGE
    ds.read(); // crc
    ds.read();
    ds.read(); // verify data
  }
}

byte find_by_string(String txt,byte* &addr){
  String t;
  ds.reset_search();
  while (ds.search(addr)) {
    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return false;
    }
    t=String(PrintBytes(addr, 8));
    if(txt==t){
      return true;
    }
  }
}

void string_to_addr(String txt,byte * addr){
  //byte addr[8];
  byte a,b;
  for (int i = 0; i < 8; i++){
    a=h2int(txt[i*3]);
    b=h2int(txt[i*3+1]);
    a=(a<<4);
    addr[i]=a+b;
  }
  return;
}


void ds_list_find() {
  byte addr[8];
  String href;

  DynamicJsonBuffer jsonBuffer;
  JsonArray& outArray = jsonBuffer.createArray();
  
  ds.reset_search();
  while (ds.search(addr)) {
    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
    }else{
      Serial.println(PrintBytes(addr, 8));
      JsonObject& itemJson = outArray.createNestedObject();
      String code=PrintBytes(addr, 8);
      itemJson["code"]=code;
      
      switch (addr[0]) {
        case 0x01:
          itemJson["href"]=false;
          itemJson["type"]="DS1990";
          break;
        case 0x10:
          href="#ds1820:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS18S20";
          break;
        case 0x12:
          href="#ds2406:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS2406";
          break;
        case 0x28:
          href="#ds1820:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS18B20";
          break;
        case 0x22:
          href="#ds1820:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS1822";
          break;
        case 0x29:
          href="#ds2408:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS2408";
          break;
        case 0x85:
          href="#ds2413:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS2413";
          break;
        case 0x26:
          href="#ds2438:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS2438";
          break;       
       case 0x20:
          href="#ds2450:code=";
          href+=code;
          itemJson["href"]=href;
          itemJson["type"]="DS2450";
          break;     
        default:
          itemJson["href"]=false;
          itemJson["type"]="DS****";
      }
    }
  }
  String out;
  outArray.printTo(out);
  server.send ( 200, "text/html", out );
}
