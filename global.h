#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef ESP12_pins
#include "ESP12_pins.h"
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <SoftwareSerial.h>

/*
class myESP8266WebServer: public ESP8266WebServer {
  public :

    myESP8266WebServer(int an): ESP8266WebServer(an) {};


    bool is_authentified(){
      Serial.println("Enter is_authentified");
      if (hasHeader("Cookie")){
        Serial.print("Found cookie: ");
        String cookie = header("Cookie");
        Serial.println(cookie);
        if (cookie.indexOf("ESPSESSIONID=1") != -1) {
          Serial.println("Authentification Successful");
          return true;
        }
      }
      Serial.println("Authentification Failed");
      return false;
    }

  //login page, also called for disconnect
  void handleLogin(){
    String msg;
    if (hasHeader("Cookie")){
      Serial.print("Found cookie: ");
      String cookie = header("Cookie");
      Serial.println(cookie);
    }
    if (hasArg("DISCONNECT")){
      Serial.println("Disconnection");
      sendHeader("Location","/login");
      sendHeader("Cache-Control","no-cache");
      sendHeader("Set-Cookie","ESPSESSIONID=0");
      //send(301);
      return;
    }
    if (hasArg("USERNAME") && hasArg("PASSWORD")){
      if (arg("USERNAME") == "admin" &&  arg("PASSWORD") == "admin" ){
        sendHeader("Location","/");
        sendHeader("Cache-Control","no-cache");
        sendHeader("Set-Cookie","ESPSESSIONID=1");
        //send(301);
        Serial.println("Log in Successful");
        return;
      }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
    }
    String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
    content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
    content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
    content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
    content += "</body></html>";
    send(200, "text/html", content);
  }

    void handleClientMW(const char* login, const char* password, String w_ip) {
      if (_currentStatus == HC_NONE) {
        WiFiClient client = _server.available();
        if (!client) {
          return;
        }

#ifdef DEBUG_ESP_HTTP_SERVER
        DEBUG_OUTPUT.println("New client");
#endif

        _currentClient = client;
        _currentStatus = HC_WAIT_READ;
        _statusChange = millis();

        if (!is_authentified()){
          handleLogin();
        }

      }
        
      handleClient();
    }

  private:
};*/


ESP8266WebServer server(80);

#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <Wire.h>


#ifdef ONE_WIRE_PORT
#include <OneWire.h>
#endif

#include <MQTTClient.h>

byte start_init = 255;
byte mqtt_connect;

#ifdef PID_TEMP
String pid_temp_in_mqtt[10];
float  pid_temp_in[10];
double  pid_in;
byte   pid_temp_in_cnt;
String pid_z_mqtt;
double  pid_z;
String pid_out_mqtt;
String pid_out_mqtt_z;
String pid_temp_in_v;
String pid_out_relay_mqtt;
byte pid_relay_status;
double  pid_out_t;
double  pid_out;
float pid_out_d;
float  pid_out_max;
double  pid_p;
double  pid_i;
double  pid_d;
byte   pid_time;

void PID_TEMP_save() {
  File configFile = SPIFFS.open("/pid.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  JsonArray& data = root.createNestedArray("pid_temp_in_mqtt");
  for (int j = 0; j < pid_temp_in_cnt; j++) {
    data.add(pid_temp_in_mqtt[j]);
  }

  root["pid_z_mqtt"] = pid_z_mqtt;
  root["pid_z"] = pid_z;
  root["pid_out_mqtt"] = pid_out_mqtt;
  root["pid_out_mqtt_z"] = pid_out_mqtt_z;
  root["pid_temp_in_v"] = pid_temp_in_v;
  root["pid_out_relay_mqtt"] = pid_out_relay_mqtt;
  root["pid_out_max"] = pid_out_max;
  root["pid_p"] = pid_p;
  root["pid_i"] = pid_i;
  root["pid_d"] = pid_d;
  root["pid_t"] = pid_time;
  root["pid_out_d"] = pid_out_d;

  root.printTo(configFile);
  configFile.close();
}
#endif

boolean initController = true;

#ifdef ONE_WIRE_PORT
OneWire ds(ONE_WIRE_PORT);
#endif

WiFiClient espClient;
MQTTClient mqtt;

struct Config_srtuct {
  String ssid;
  String password;
  String mqtt_server;
  char* mac;
  int temp_interval;
  String mqtt_user;
  String mqtt_pass;
  String mqtt_name;
  String update_dir;
  String update_file;
  char* www_login;
  char* www_password;
} config;

byte last_sec, t_sec;
boolean debug_mode = false;

IPAddress myIP;
//mqtt output
String mqtt_output[30];
byte mqtt_count = 0;

//ds temp
struct ds_data {
  byte addr[8];
  String mqtt;
};

//ds 2406/2408
struct ds_24 {
  byte addr[8];
  String mqtt;
  byte role;
  byte invert_;
  byte start;
  byte last_satate;
#ifdef UN_drebizg
  byte last_satate_dreb[UN_drebizg];
#endif
  byte last_out;
};

//ds 2438
struct ds_2438 {
  byte addr[8];
  String mqtt_v;
  String mqtt_t;
  int intrval;
};

//ds 2450
struct ds_2450 {
  byte addr[8];
  String mqtt;
  int intrval;
};

byte ds_temp_cnt = 0;
ds_data ds_temp[5];

byte ds_24_cnt = 0;
ds_24 ds24_data[5];

byte ds_2438_cnt = 0;
ds_2438 ds2438_data[5];

byte ds_2450_cnt = 0;
ds_2450 ds2450_data[5];

boolean isStatic(String filename) {
  if (filename.endsWith(".css")) return  true;
  else if (filename.endsWith(".js")) return  true;
  else if (filename.endsWith(".png")) return true;
  else if (filename.endsWith(".gif")) return true;
  else if (filename.endsWith(".jpg")) return true;
  else if (filename.endsWith(".jpeg")) return true;
  else if (filename.endsWith(".ico")) return true;
  return false;
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

byte sootv_addr(byte *addr1, byte *addr2) {
  for (int i = 0; i < 8; i++) {
    if (addr1[i] != addr2[i]) return false;
  }
  return true;
}

String PrintBytes(uint8_t* addr, uint8_t count) {
  String out;
  for (uint8_t i = 0; i < count; i++) {
    //out+=addr[i];
    //out+='-';
    out += String(addr[i] >> 4, HEX);
    out += String(addr[i] & 0x0f, HEX);
    if (i < count - 1)out += '-';
  }
  return out;
}
#endif
