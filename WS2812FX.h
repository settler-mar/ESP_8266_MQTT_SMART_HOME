#include <Adafruit_NeoPixel.h>

#define DEFAULT_COLOR 0x00FF59

String ws2812_mqtt_in;
String ws2812_mqtt_out;
byte   ws2812_led_d;
byte   ws2812_led_z;
int    ws2812_step_d;
int    ws2812_micro_step_d;
byte   ws2812_dir;
byte   ws2812_state;

const byte  EFFECT_CNT = 6;

int ws_led_count = WS_LED_COUNT;
float DK=255/WS_LED_COUNT;

#define BASE_COLORS_CNT 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(WS_LED_COUNT, WS_PIN, NEO_GRB + NEO_KHZ800);
const uint8_t BASE_COLORS[]= { pixels.Color(255,0,0),0x00FF00,0x0000FF,0xFFFFFF,0xFFFF00,0x00FFFF,0xFF00FF,0x400080,0xFF3000,0x800000};

uint8_t ws_picels[WS_LED_COUNT];
int ws_step=0; //для большого шага расчетная велчина от времени
int ws_micro_step=0;
int ws_step_cnt=0;//счетчик большого шага
int ws_micro_step_cnt=0;//счетчик мелкого шага (примерно 0.5 от большого)
int ws_effect=5;
int ws_sel_color=0;
int ws_sel_color_cnt=0;
int ws_effect_cnt=0; //счетчик внутри эффекта
byte ws_effect_cnt2=0; //счетчик внутри эффекта

// some common colors
#define BLACK   0x000000
#define WHITE   0xFFFFFF
/*#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF
#define WHITE   0xFFFFFF
#define YELLOW  0xFFFF00
#define CYAN    0x00FFFF
#define MAGENTA 0xFF00FF
#define PURPLE  0x400080
#define ORANGE  0xFF3000*/

void ws_efect(boolean is_big);
void ws_efect_run(),
     ws_efect_add(),
     ws_efect_flash_raduga(boolean is_big),
     ws_efect_raduga(),
     ws_efect_rnd(),
     ws_efect_candy(boolean is_big);

void ws2812_calc_params(){
  if(ws2812_led_z<1){
    ws2812_led_z=1;
  }
  ws_led_count = WS_LED_COUNT/ws2812_led_z;
  DK=255/ws_led_count;
  ws2812_led_d=ws2812_led_d % ws_led_count;

  mqtt_add_out(ws2812_mqtt_in);
}

uint32_t color_wheel_br(int pos,byte br) {
  byte r = 0;
  byte g = 0;
  byte b = 0;
  pos=pos % 255;
  pos = pos * 3;
  if (pos<=255){
    b=pos;
    r=255-pos;
  }else if(pos<510){
    pos=pos-255;
    g=pos;
    b=255-pos;
  }else{
    pos=pos-510;
    r=pos;
    g=255-pos;
  }
  if(br<255){
    r=br*r/255;
    g=br*g/255;
    b=br*b/255;
  }
  return pixels.Color(r,g,b);
}

uint32_t color_wheel(int pos) {
 return color_wheel_br(pos,255);
}

/*

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint32_t c) {
  RESET_RUNTIME;
  _segments[0].colors[0] = c;
  setBrightness(_brightness);
}
*/

void WSsetPixelColor(int i,uint32_t color){
  i=(ws2812_led_d + i) % ws_led_count;
  if(ws2812_dir){
    i=ws_led_count-i-1;
  }
  Serial.println(i);
  pixels.setPixelColor(i, color);
  if(ws2812_led_z>1){
    for(int j=1;j<ws2812_led_z;j++){
      i=i+ws_led_count;
      pixels.setPixelColor(i, color);
    }
  }
}

void ws2812_load(){
  ws2812_mqtt_in = "/ws2812/in";
  ws2812_mqtt_out = "/ws2812/out";
  ws2812_led_d = 0;
  ws2812_led_z = 1;
  ws2812_step_d=20;
  ws2812_micro_step_d=100;
  ws2812_dir=0;
  
  DynamicJsonBuffer jsonBuffer;
  
  File configFile = SPIFFS.open("/ws2812fx.json", "r");
  if (!configFile) {
    Serial.println("Failed to open ANALOG config file for reding");
    return;
  }
  
  String line = configFile.readStringUntil('\n');
      //Serial.println(line);
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
   Serial.println("parseObject() failed");
   return;
  }

  ws2812_mqtt_in = root["mqtt_in"].as<String>();
  ws2812_mqtt_out = root["mqtt_out"].as<String>();
  ws2812_led_d = StringToInt(root["led_d"].as<String>());
  ws2812_led_z = StringToInt(root["led_z"].as<String>());
  ws2812_step_d = StringToInt(root["step_d"].as<String>());
  ws2812_micro_step_d = StringToInt(root["micro_step_d"].as<String>());
  ws2812_dir = StringToInt(root["dir"].as<String>());
}

void ws2812_off(){
  for(int i=0;i<WS_LED_COUNT;i++){
    pixels.setPixelColor(i,pixels.Color(0,0,0)); 
  }
  pixels.show();
  ws2812_state=0;
}

void ws2812_on(){
  ws2812_state=1;
  ws_step=0;
  ws_step_cnt=0;
  ws_micro_step_cnt=0;
  ws_sel_color=0;
  ws_sel_color_cnt=0;
  ws_effect_cnt=0; //счетчик внутри эффекта
  ws_effect_cnt2=0; //счетчик внутри 
}

void ws2812_init(){
  pixels.begin(); // This initializes the NeoPixel library 
  for(int i=0;i<WS_LED_COUNT;i++){
    ws_picels[i]=pixels.Color(0,0,0);
    pixels.setPixelColor(i, ws_picels[i]); 
  }
  //pixels.setBrightness(255);
  pixels.show();

  for(int i=0;i<WS_LED_COUNT;i++){
    pixels.setPixelColor(i,pixels.Color(0,200,0)); 
    delay(3000/WS_LED_COUNT);
    pixels.show();
  }
  delay(200);
  ws2812_off();
    
  ws2812_load();
  ws2812_calc_params();
  //mqtt_add_out("ws2812");
}

void ws2812_save(){
  File configFile = SPIFFS.open("/ws2812fx.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["mqtt_in"]=ws2812_mqtt_in;
  root["mqtt_out"]=ws2812_mqtt_out;
  root["led_d"]=ws2812_led_d;
  root["led_z"]=ws2812_led_z;
  root["step_d"]=ws2812_step_d;
  root["micro_step_d"]=ws2812_micro_step_d;
  root["dir"]=ws2812_dir;
 

  root.printTo(configFile);
  configFile.close();
}
    
void ws2812fx_config(){
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  if (server.hasArg("mqtt_in")){
    ws2812_mqtt_in =server.arg("mqtt_in");
    ws2812_mqtt_out = server.arg("mqtt_out");
    ws2812_led_d = StringToInt(server.arg("led_d"));
    ws2812_led_z = StringToInt(server.arg("led_z"));
    ws2812_step_d =  StringToInt(server.arg("step_d"));
    ws2812_micro_step_d =  StringToInt(server.arg("micro_step_d"));
    ws2812_dir  =  StringToInt(server.arg("dir"));
  }
  
  root["mqtt_in"]=ws2812_mqtt_in;
  root["mqtt_out"]=ws2812_mqtt_out;
  root["led_d"]=ws2812_led_d;
  root["led_z"]=ws2812_led_z;
  root["step_d"]=ws2812_step_d;
  root["micro_step_d"]=ws2812_micro_step_d;
  root["dir"]=ws2812_dir;

  if (server.hasArg("mqtt_in")){
    ws2812_save();
    ws2812_calc_params();
    root["msg"]="Data updated.";
  }

  String out;
  root.printTo(out);
  server.send ( 200, "text/html", out );
}

void ws2812_run(){
  if(!ws2812_state)return;
  int t_sec = millis() % 32000 ;
  int t;
  t=t_sec/ws2812_micro_step_d;
  if(t==ws_micro_step)return;
  ws_micro_step=t;
  
  t=t/ws2812_step_d;
  if(ws_step!=t){
    ws_step!=t;
    //отрисовка большого шага если надо
    ws_step_cnt++;
    ws_micro_step_cnt=0;
    ws_efect(true);
  }else{
     //отрисовка микро шага если надо и если не отрисовывали основной
     ws_micro_step_cnt++;
     ws_efect(false); 
  }
  
/*    ws_step=t;
    //отрисовка большого шага если надо
    ws_step_cnt++;
    ws_micro_step_cnt=0;
    ws_efect(true);
  }else{
    t=t_sec/ws2812_micro_step_d;
    if(t!=ws_micro_step){
      ws_micro_step=t;
       //отрисовка микро шага если надо и если не отрисовывали основной
       ws_micro_step_cnt++;
       ws_efect(false); 
    }
  }
    
  /*for(int i=0;i<WS_LED_COUNT;i++){
    WSsetPixelColor(i,pixels.Color(0,200,0)); 
    delay(400);
    pixels.show();
  }*/
  //delay(400);
}

void ws2812_addLast(uint32_t color, boolean circle = false){
  byte i;
  uint32_t old = ws_picels[0];
  for(i=1;i<ws_led_count;i++){
    ws_picels[i-1]=ws_picels[i];
    WSsetPixelColor(i-1, ws_picels[i-1]); 
  }
  ws_picels[i-1]=color;
  WSsetPixelColor(i-1, ws_picels[i-1]); 

  pixels.show();
}


void ws2812_addFirst(uint32_t color, boolean circle = false){
  byte i;
  for(i=ws_led_count-1;i>0;i--){
    ws_picels[i]=ws_picels[i-1];
    WSsetPixelColor(i, ws_picels[i]); 
  }
  ws_picels[i]=color;
  WSsetPixelColor(i, ws_picels[i]); 

  pixels.show();
}

void ws_efect(boolean is_big){
  if(ws_effect==5){
    ws_efect_candy(is_big);
    return;
  }else if(ws_effect==0){
    ws_efect_run();
    return;    
  }else if(ws_effect==1){
    //if(is_big){
      ws_efect_add();
    //}
    return;    
  }else if(ws_effect==2){
    ws_efect_raduga();
    return;    
  }else if(ws_effect==3){
     ws_efect_flash_raduga(is_big);
    return;    
  }else if(ws_effect==4){
    ws_efect_rnd();
    return;    
  }
}

void ws_efect_run(){
  uint32_t color;
  int i;
  color=color_wheel(ws_sel_color);

  i=ws_step_cnt % ws_led_count;
  ws_sel_color=ws_sel_color+8;
  if(ws_sel_color>255){
    ws_sel_color=0;
  }
  
  WSsetPixelColor(i, color);
  if(i==0){
    i=ws_led_count-1;
  }else{
    i=i-1;
  }
  WSsetPixelColor(i, BLACK);
  pixels.show();
}

void ws_efect_add(){
  uint32_t color;
  int i;

  ws_sel_color=(ws_led_count-ws_sel_color_cnt+1)*DK;
  color=color_wheel(ws_sel_color);
  
  i=ws_step_cnt % (ws_led_count-ws_sel_color_cnt);
  if(i==ws_led_count-ws_sel_color_cnt-1){
    ws_sel_color_cnt++;
    ws_step_cnt=0;
    if(ws_sel_color_cnt==ws_led_count-1){
      ws_sel_color_cnt=0;
    }
  }

  ws_sel_color=ws_sel_color+8;
  if(ws_sel_color>255){
    ws_sel_color=-1;
  }
  
  WSsetPixelColor(i, color);
  if(i==0){

  }else{
    i=i-1;
    WSsetPixelColor(i, BLACK);
  }

  pixels.show();
}

void ws_efect_raduga(){
  byte i,col;
  ws_sel_color=ws_sel_color+1;
  if(ws_sel_color>255){
    ws_sel_color=0;
  }
  
  for(i=0;i<ws_led_count;i++){
    col=ws_sel_color+i*DK;
    WSsetPixelColor(i, color_wheel(col)); 
  }
  
  pixels.show();
}

void ws_efect_flash_raduga(boolean is_big){
  uint32_t color;
  int i;
  
  color=color_wheel(ws_sel_color);

  i=ws_step_cnt % ws_led_count;

  if(is_big){
    WSsetPixelColor(i, WHITE);
    if(i==0){
      WSsetPixelColor(ws_led_count-1, color);
      ws_sel_color=ws_sel_color+145;
      if(ws_sel_color>255){
        ws_sel_color=ws_sel_color-255;
      }
    }else{
      i=i-1;
      WSsetPixelColor(i, color);
    }
  }else{
    if(ws_micro_step_cnt%2==0){
      WSsetPixelColor(i, BLACK);
    }else{
      WSsetPixelColor(i, WHITE);
    }
  }
  pixels.show();
}

void ws_efect_rnd(){
  int i=random(0,ws_led_count);
  uint32_t color=color_wheel(ws_sel_color);
  WSsetPixelColor(i, color);
  ws_sel_color=ws_sel_color+145;
  if(ws_sel_color>255){
    ws_sel_color=ws_sel_color-255;
  }
  pixels.show();
}

bool ws2812_processJson(String message) {
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(message);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }

  //root.prettyPrintTo(Serial);
  if (root.containsKey("state")) {
    if (strcmp(root["state"], "ON") == 0) {
      ws2812_on();
    }
    else if (strcmp(root["state"], "OFF") == 0) {
      ws2812_off();
    }
  }

  return true;
}

void ws2812_mqtt(String mqtt_str,String val_s){
  if(ws2812_mqtt_in!=mqtt_str)return;
  if(val_s=="255" || val_s=="100"){
    ws2812_on();
    ws2812_calc_params();
    return;
  }
  if(val_s=="0"){
    ws2812_off();
    return;
  }

  if (!ws2812_processJson(val_s)) {
    return;
  }
  ws2812_calc_params();
}

//EFFECT Candy Cane
void ws_efect_candy(boolean is_big){
  uint32_t color;

  color=color_wheel_br(ws_sel_color,ws_effect_cnt2*255/ws2812_step_d);  
  WSsetPixelColor(ws_effect_cnt, color);
  pixels.show();

  if(ws_effect_cnt2>=ws2812_step_d){
    ws_effect_cnt2=0;
    ws_effect_cnt++;
  }else{
    ws_effect_cnt2++;
  }
  if(ws_effect_cnt>=ws_led_count){
    ws_effect_cnt=0;
    ws_sel_color=ws_sel_color+48;
    if(ws_sel_color>255){
      ws_sel_color=0;
    }
  }
}

