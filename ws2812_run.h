#include <Adafruit_NeoPixel.h>

byte ws2812_run_step;
byte ws2812_run_status,ws2812_run_mode = 0;
byte ws2812_run_up_status = 0;
byte ws2812_run_dwn_status = 0;
int ws2812_run_color = 0;

//настройки
int ws2812_run_max_analog=140;
int ws2812_run_min_brightness=60;
int ws2812_run_off_analog=160;
int ws2812_run_wait_time=5;
int ws2812_run_dim_step=5;
String ws2812_run_mqtt_up = "/koreidor/move_up";
String ws2812_run_mqtt_dwn = "/koreidor/move_down";
String ws2812_run_mqtt_analog = "/koreidor/analog";

#define ws2812_run_color_cnt 5
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN_ws, NEO_GRB + NEO_KHZ800);

uint32_t ws_r_picels[NUMPIXELS];

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}



void ws2812_run_init(){
  strip.begin(); // This initializes the NeoPixel library
  ws2812_run_step=4;

  pinMode(ws2812_run_up_pin, INPUT);
  pinMode(ws2812_run_dwn_pin, INPUT);
  #ifdef ws2812_run_analog_pin
    pinMode(ws2812_run_analog_pin, INPUT);
  #endif
  
  for(byte i=0;i<NUMPIXELS;i++){
    ws_r_picels[i]=strip.Color(0,0,0, 0 );
  }
  strip.setBrightness(20);
  strip.show();

  DynamicJsonBuffer jsonBuffer;
  File configFile = SPIFFS.open("/ws_run.json", "r");
  if (!configFile) {
    Serial.println("Failed to open WS RUN config file for reding");
    return;
  }
  String line = configFile.readStringUntil('\n');
    //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
  }else{
     ws2812_run_mqtt_up = root["mqtt_up"].as<String>();
     ws2812_run_mqtt_dwn = root["mqtt_dwn"].as<String>();
     ws2812_run_mqtt_analog = root["mqtt_analog"].as<String>();
     ws2812_run_max_analog = StringToInt(root["max_analog"].as<String>());
     ws2812_run_min_brightness = StringToInt(root["min_brightness"].as<String>());
     ws2812_run_off_analog = StringToInt(root["off_analog"].as<String>());
     ws2812_run_wait_time = StringToInt(root["wait_time"].as<String>());
     ws2812_run_dim_step = StringToInt(root["dim_step"].as<String>());
  }
}

void ws2812_run_addLast(uint32_t color){
  byte i;
  for(i=1;i<NUMPIXELS;i++){
    ws_r_picels[i-1]=ws_r_picels[i];
    strip.setPixelColor(i-1, ws_r_picels[i-1]); 
  }
  ws_r_picels[i-1]=color;
  strip.setPixelColor(i-1, ws_r_picels[i-1]); 

  strip.show();
}


void ws2812_run_addFirst(uint32_t color){
  byte i;
  for(i=NUMPIXELS-1;i>0;i--){
    ws_r_picels[i]=ws_r_picels[i-1];
    strip.setPixelColor(i, ws_r_picels[i]); 
  }
  ws_r_picels[i]=color;
  strip.setPixelColor(i, ws_r_picels[i]); 

  strip.show();
}

uint32_t ws2812_run_rnd_color(byte i){
  int r,g,b;
  switch (ws2812_run_color) {
    case 0://сине-березовый
      r=0;
      g=random(0,80);
      b=random(100,255);
      break;    
    case 1://сине-фиолетовый
      r=random(0,80);
      g=0;
      b=random(100,255);
      break;
    case 2://зеленый-белый
      r=random(0,50);
      g=random(180,255);
      b=r;
      break;
    case 3://красно-оранжевый
      r=random(100,255);
      g=random(0,150);
      b=0;
      break;
    case 4://сине-белый
      r=random(0,100);
      g=r;
      b=random(150,255);;
      break;
  }
  if(i>ws2812_run_dim_step){
    return strip.Color(lowByte(r),lowByte(g),lowByte(b));
  }else{
    r=r*i/ws2812_run_dim_step;
    g=g*i/ws2812_run_dim_step;
    b=b*i/ws2812_run_dim_step;
    return strip.Color(lowByte(r),lowByte(g),lowByte(b));
  }
}

void ws2812_run_do(byte t_sec){
  byte i,tmp;

//ws2812_run_status,ws2812_run_mode

  if(ws2812_run_status==1){
    if(ws2812_run_step==0){
      #ifdef ws2812_run_analog_pin
        int level= analogRead(ws2812_run_analog_pin);
        mqtt_pub(ws2812_run_mqtt_analog,String(level));
        if(level>ws2812_run_off_analog){
          ws2812_run_status=0;
          return;
        }
        if(level>ws2812_run_max_analog){
          level=ws2812_run_max_analog;
        }

        byte brightness = (255-ws2812_run_min_brightness)*(ws2812_run_max_analog-level)/ws2812_run_max_analog;
        strip.setBrightness(255-brightness);
      #else
        strip.setBrightness(ws2812_run_min_brightness);
      #endif
    }
    
    if(ws2812_run_mode==1){
      ws2812_run_addFirst(ws2812_run_rnd_color(ws2812_run_step));
    }else{
      ws2812_run_addLast(ws2812_run_rnd_color(ws2812_run_step));
    }
    
    ws2812_run_step++;
    if(ws2812_run_dim_step<ws2812_run_step){
      ws2812_run_step=0;
      ws2812_run_status=2;
    }
  }

  if(ws2812_run_status==2){
    if(ws2812_run_mode==1){
      ws2812_run_addFirst(ws2812_run_rnd_color(255));
    }else{
      ws2812_run_addLast(ws2812_run_rnd_color(255));
    }
    
    if(t_sec % T_PERIOD == 0 && !ws2812_run_dwn_status && !ws2812_run_up_status){
      ws2812_run_step++;
    }
    if(ws2812_run_step>ws2812_run_wait_time){
      ws2812_run_step=0;
      ws2812_run_status=3;
    }
  }

  if(ws2812_run_status==3){
    ws2812_run_step++;
    if(ws2812_run_mode==1){
      ws2812_run_addFirst(ws2812_run_rnd_color(ws2812_run_dim_step-ws2812_run_step+1));
    }else{
      ws2812_run_addLast(ws2812_run_rnd_color(ws2812_run_dim_step-ws2812_run_step+1));
    }
      
    if(ws2812_run_dim_step<ws2812_run_step){
      ws2812_run_step=0;
      ws2812_run_status=4;
    }
  }
  
  if(ws2812_run_status==4){
    ws2812_run_step++;
    if(ws2812_run_mode==1){
      ws2812_run_addFirst(strip.Color(0,0,0 ));
    }else{
      ws2812_run_addLast(strip.Color(0,0,0 ));
    }
      
    if(NUMPIXELS<ws2812_run_step){
      ws2812_run_step=0;
      ws2812_run_status=0;
    }
  }

  tmp = digitalRead(ws2812_run_up_pin);
  if(ws2812_run_up_status!=tmp){
    ws2812_run_up_status=tmp;

    mqtt_pub(ws2812_run_mqtt_up,String(ws2812_run_up_status));
    
    if(tmp){
      if(ws2812_run_status==0){
        ws2812_run_step=0;
        ws2812_run_mode=1;
        ws2812_run_status=1;
        ws2812_run_color++;
      }
      if(ws2812_run_status==2){
        ws2812_run_step=0;
      }
    }
  }

  tmp = digitalRead(ws2812_run_dwn_pin);
  if(ws2812_run_dwn_status!=tmp){
    ws2812_run_dwn_status=tmp;

    mqtt_pub(ws2812_run_mqtt_dwn,String(ws2812_run_dwn_status));
    
    if(tmp){
      if(ws2812_run_status==0){
        ws2812_run_mode=2;
        ws2812_run_status=1;
        ws2812_run_step=0;
        ws2812_run_color++;
      }
      if(ws2812_run_status==2){
        ws2812_run_step=0;
      }
    }
  }
  if(ws2812_run_color_cnt<=ws2812_run_color){
    ws2812_run_color=0;
  }
  /*
    byte ws2812_run_up_status = 0;
    byte ws2812_run_dwn_status = 0;
    #define ws2812_run_up_pin D0
    #define ws2812_run_dwn_pin D1
  */
}

void ws_run_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("max_analog")){
     ws2812_run_mqtt_up = server.arg("mqtt_up");
     ws2812_run_mqtt_dwn = server.arg("mqtt_dwn");
     ws2812_run_mqtt_analog = server.arg("mqtt_analog");
     ws2812_run_max_analog = StringToInt(server.arg("max_analog"));
     ws2812_run_min_brightness = StringToInt(server.arg("min_brightness"));
     ws2812_run_off_analog = StringToInt(server.arg("off_analog"));
     ws2812_run_wait_time = StringToInt(server.arg("wait_time"));
     ws2812_run_dim_step = StringToInt(server.arg("dim_step"));
  }
  
  root["max_analog"] = ws2812_run_max_analog;
  root["min_brightness"]=ws2812_run_min_brightness;
  root["off_analog"]=ws2812_run_off_analog;
  root["wait_time"]=ws2812_run_wait_time;
  root["mqtt_up"]=ws2812_run_mqtt_up;
  root["mqtt_dwn"]=ws2812_run_mqtt_dwn;
  root["mqtt_analog"]=ws2812_run_mqtt_analog;
  root["dim_step"]=ws2812_run_dim_step;

  if (server.hasArg("max_analog")){
    File configFile = SPIFFS.open("/ws_run.json", "w");
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

