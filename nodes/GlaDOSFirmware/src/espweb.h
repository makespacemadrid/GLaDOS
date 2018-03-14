#ifndef ESPWIFI
#define ESPWIFI


ESP8266WebServer updateServer(8080);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

ESP8266WebServer fileServer(80);
//holds the current upload
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

String getContentType(String filename){
  if(fileServer.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = fileServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(fileServer.uri() != "/edit") return;
  HTTPUpload& upload = fileServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
  }
}

void handleFileDelete(){
  if(fileServer.args() == 0) return fileServer.send(500, "text/plain", "BAD ARGS");
  String path = fileServer.arg(0);
  if(path == "/")
    return fileServer.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return fileServer.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  fileServer.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate(){
  if(fileServer.args() == 0)
    return fileServer.send(500, "text/plain", "BAD ARGS");
  String path = fileServer.arg(0);
  if(path == "/")
    return fileServer.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return fileServer.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return fileServer.send(500, "text/plain", "CREATE FAILED");
  fileServer.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if(!fileServer.hasArg("dir")) {fileServer.send(500, "text/plain", "BAD ARGS"); return;}

  String path = fileServer.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  fileServer.send(200, "text/json", output);
}


//File Server
void startHttpFileServer(uint16_t port)
{

  //fileServer = ESP8266WebServer(port);

  //SERVER INIT
  //list directory
  fileServer.on("/list", HTTP_GET, handleFileList);
  //load editor
  fileServer.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) fileServer.send(404, "text/plain", "FileNotFound");
  });
  //create file
  fileServer.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  fileServer.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  fileServer.on("/edit", HTTP_POST, [](){ fileServer.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  fileServer.onNotFound([](){
    if(!handleFileRead(fileServer.uri()))
      fileServer.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  fileServer.on("/all", HTTP_GET, [](){
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    fileServer.send(200, "text/json", json);
    json = String();
  });
  fileServer.begin();
  Serial.println("File Server started open http://IP/edit");
}


//Firmware updater

void startHttpUpdater(uint16_t port)
{
    //updateServer = ESP8266WebServer(port);
    updateServer.on("/", HTTP_GET, [](){
      updateServer.sendHeader("Connection", "close");
      updateServer.sendHeader("Access-Control-Allow-Origin", "*");
      updateServer.send(200, "text/html", serverIndex);
    });
    updateServer.on("/update", HTTP_POST, [](){
      updateServer.sendHeader("Connection", "close");
      updateServer.sendHeader("Access-Control-Allow-Origin", "*");
      updateServer.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = updateServer.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    updateServer.begin();
}

#endif // ESPWIFI
