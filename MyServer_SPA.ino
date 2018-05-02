#include <string.h>
#include "configuration.h"
#include "global.h"
#include "helpers.h"
#include "mqtt.h"

#include "config.h"


#ifdef ESP_auth_def
  #include "auth.h"
#endif

#ifdef wifi_ota
  #include <ArduinoOTA.h>
  boolean wifi_ota_run=false;
#endif

#ifdef BMP180
  #include <Adafruit_Sensor.h>
  #include "Adafruit_BMP085.h"
  #include "BMP180.h"
#endif

#ifdef DHT11_PIN
  #include <DHT.h>
  #include <DHT_U.h>
  #include "DHT11_.h"
#endif

//#ifdef FILE_EDIT
  #include "file_edit.h"
//#endif

#ifdef RC433_PORT
  #include "rc433.h"
#endif


#ifdef PID_TEMP
  #include "PID_TEMP.h"
#endif

#ifdef analog_pin
  #include "analog.h"
#endif

#ifdef wifi_firmware_update
  #include "wifi_firmware_update.h"
#endif

#ifdef RDM6300
  #include "rdm6300.h"
#endif

#ifdef ws2812_run
  #include "ws2812_run.h"
#endif

#ifdef WS_PIN
  #include "WS2812FX.h"
#endif

#ifdef ONE_WIRE_PORT
  #include "onewire_help.h"
  #include "ds18b20.h"
  #include "DS2406.h"
  #include "ds2408.h"
  #include "ds2438.h"
  #include "ds2450.h"
#endif

#ifdef MCP
  #include "mcp23017.h";
#endif

#ifdef Alarm_mode
  #include "Alarm_modem.h"
#endif

#include "server.h"

void setup(void){
  Serial.begin(UART_SPEAD);
  WiFi.mode(WIFI_OFF);
  delay(1500);

  String n1=String(ESP.getChipId());
  WiFi.hostname(n1);
  
  //ESP.restart();

  Serial.print("Chip id ");
  Serial.println(ESP.getChipId());

  Serial.print("Chip size ");
  Serial.println(ESP.getFlashChipRealSize());

  Serial.print("Chip Speed ");
  Serial.println(ESP.getFlashChipSpeed());

  //ADC_MODE(ADC_VCC);
  Serial.print("Vcc ");
  Serial.println(ESP.getVcc());


  #ifdef DEBUG_ENABLE
    pinMode(init_debug_port, INPUT);
    debug_mode=false;
    if(digitalRead(init_debug_port)==HIGH){
      debug_mode=true;
      Serial.println( "HIGH");
    }else{
      debug_mode=false;
      Serial.println( "LOU");
    };
  #endif
  
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }else{
    FSInfo fs_info;
    SPIFFS.info(fs_info);
   
    Serial.print("FS size total ");
    Serial.println(formatBytes(fs_info.totalBytes));
    
    Serial.print("FS size used ");
    Serial.println(formatBytes(fs_info.usedBytes));

    Serial.print("FS max path length ");
    Serial.println(fs_info.maxPathLength);
  }

  //SPIFFS.format();
  
  if(!loadConfig()){
    defaultConfig();
    saveConfig();
  };

  #ifdef DEF_CONFIG_ON_LOAD
    defaultConfig();
  #endif

  #ifdef ONE_WIRE_PORT
    loadOneWere();
  #endif
  
  //WiFi.begin ( config.ssid, config.password);
  Serial.println("");

  subsribe=0;

  #ifdef DHT11_PIN
    DHT_init();
  #endif
  
  #ifdef BMP180
    BMP180_init();
  #endif

  #ifdef analog_pin
    analog_init();
  #endif
  
  #ifdef RDM6300
    RDM6300_init();
  #endif
  
  #ifdef PID_TEMP
    PID_TEMP_init();
  #endif

  #ifdef ws2812_run
    Serial.println("ws2812_run_init");
    ws2812_run_init();
  #endif
  
  #ifdef WS_PIN
    ws2812_init();
  #endif

  if(debug_mode){
    WiFi.mode(WIFI_AP);
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP("ESPhome", "thereisnospoon");
  
    myIP = WiFi.softAPIP();
    Serial.println("SSID:          ESPhome");
    Serial.println("PASSWORD:      thereisnospoon");
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    // Set up mDNS responder:
    // - first argument is the domain name, in this example
    //   the fully-qualified domain name is "esp8266.local"
    // - second argument is the IP address to advertise
    //   we send our IP address on the WiFi network
    /*if (!MDNS.begin("ESPhome")) {
      Serial.println("Error setting up MDNS responder!");
      while(1) { 
        delay(1000);
      }
    }*/
  }else{
    WiFi.mode(WIFI_STA);
    // Wait for connection
    Serial.print("Connected WIFI");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(config.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print ( "MAC address: " );
    config.mac=GetMacAddress();
    Serial.println ( GetMacAddress() );
    myIP = WiFi.localIP();
  
    mqtt_init();

  };
  #ifdef PID_TEMP
    start_init = t_sec>UN_drebizg*3);
  #else
    start_init=15;
  #endif
  
  server_init();
  initController=false;

  #ifdef ESP_auth_def
    auth_init();
  #endif

  #ifdef RC433_PORT
    rc433_init();
  #endif

  #ifdef MCP
    mcp_setup();
  #endif

  #ifdef wifi_ota
    #include <ArduinoOTA.h>
      ArduinoOTA.onStart([]() {
      Serial.println("OTA start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nOTA end");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  
  #endif
}

void loop(void){ 
  if(!debug_mode){
    t_sec = millis() / (1000 / T_PERIOD);
    
    if(t_sec!=last_sec){
      mqtt.loop();
      mqtt_connect=mqtt.connected();
      
      if ((t_sec % T_PERIOD == 0) && !mqtt_connect) {
        mqtt_init();
      }else{
        if(mqtt_connect && subsribe==0){
          mqtt_subscribe();
        }
      }

      if(start_init){
        start_init=start_init-1;
      }

      #ifdef ONE_WIRE_PORT
        volt_read(t_sec);
        volt_2450_read(t_sec);
      #endif


      #ifdef PID_TEMP
        PID_analiz(t_sec);
      #endif

      
      byte temp_period=(t_sec % (config.temp_interval * T_PERIOD));
      #ifdef ONE_WIRE_PORT
        if(temp_period % T_PERIOD == 0){
          if (mqtt_connect) {
            mqtt_temp(temp_period / T_PERIOD);
          }
          calculate_temp(temp_period / T_PERIOD);
        }
      #endif

       
      if(temp_period==config.temp_interval * T_PERIOD-2){
        if (mqtt_connect) {
          //mqtt_temp();
          #ifdef DHT11_PIN
            DHT11_read();
          #endif
          #ifdef BMP180
            BMP180_read();
          #endif 
        }
      }

      #ifdef analog_pin
        temp_period=(t_sec % (analog_intrval*T_PERIOD));
        if(temp_period==0){
          analog_read();
        }
      #endif

      #ifdef RDM6300
        RDM6300_render();
      #endif

      #ifdef ws2812_run
        ws2812_run_do(t_sec);
      #endif
      
      #ifdef RC433_PORT
        rc433_do();
      #endif

      #ifdef MCP
        mcp_loop();
      #endif
      
      //Serial.println(t_sec);
      mqtt_24();    
      //Serial.println();
      last_sec=t_sec;

      server.handleClient();
    }

    #ifdef WS_PIN
      ws2812_run();
    #endif
  }else{
    server.handleClient();
  }
  #ifdef wifi_ota
    if(wifi_ota_run){
      ArduinoOTA.handle();
    }
  #endif   
}
