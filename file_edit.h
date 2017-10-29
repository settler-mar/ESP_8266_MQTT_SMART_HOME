File fsUploadFile;

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
  }
}

void handleFileDelete(){
  if(server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if(path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!server.hasArg("dir")) {server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    isDir=(entry.size()==0);
    
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"size\":\"";
    output += formatBytes(entry.size());
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  server.send(200, "text/json", output);
}
