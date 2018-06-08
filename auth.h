String auth_ip ="";
//Check if header is present and correct
bool is_authentified(){

  String addr = server.client().remoteIP().toString();
  if(addr==config.mqtt_server)return true;

  if(auth_ip==addr)return true;

  /*if (server.hasHeader("Cookie")){   
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful *");
      return true;
    }
  }*/
  return false;  
}


void handleLogin(String uri){
  if(uri=="/main"){
    int method = server.method();
    if(method == HTTP_POST && server.hasArg("user") && server.hasArg("password")){
      Serial.println("Data_in:");
      Serial.print("user: ");
      Serial.println(server.arg ( "user" ));
      Serial.print("password: ");
      Serial.println(server.arg ( "password" ));
      Serial.println("Data_wait:");
      Serial.print("user: ");
      Serial.println(config.www_login);
      Serial.print("password: ");
      Serial.println(config.www_password );

      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
  
      if(
        (server.arg ("user")==config.www_login || config.www_login=="") && 
        (server.arg ( "password" ) == config.www_password || config.www_password=="")
      ){
          root["msg"]="Login ok. Please reload page.";
          root["reload"]=true;
          auth_ip=server.client().remoteIP().toString();
      }else{
          root["msg"]="Login error.";
      }

      String out;
      root.printTo(out);
      server.send ( 200, "text/html", out );
    }
  }
  
  if(!handleFileRead("/index.html")){
    server.send(404, "text/plain", "FileNotFound");
  }
}

void loginRoute(){
  String uri = server.uri();
  if(isStatic(uri)){
    if(!handleFileRead(uri)){
      server.send(404, "text/plain", "FileNotFound");
    }
    return;
  }
 
  if(uri.endsWith(".json")){
    if(!handleFileRead("/pages/login.json")){
      server.send(404, "text/plain", "FileNotFound");
    }
    return;
  }

  Serial.print("login route: ");
  Serial.println(uri);

  handleLogin(uri);
  return;
}
