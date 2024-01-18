#ifndef WEBHELPERS_H
#define WEBHELPERS_H

#include "qtcompat.h"
#include "nodeclient.h"

nodeWifiMode currentWifiMode;
bool wifiAPMode = false;

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

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  fileServer = ESP8266WebServer(port);

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
    updateServer = ESP8266WebServer(port);
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

void esp_init(nodeSettings* s)
{
    Serial.println("Init wifi module...");
//    WiFi.mode(WIFI_STA);
//    WiFi.disconnect();
    bool overmindFound = false;
    currentWifiMode = s->wifiMode;
    if(s->allowWifiOverride)
    {
        Serial.print("Searching override network... ");
        int n = WiFi.scanNetworks();
        Serial.print(n);
        Serial.println(" networks found.");
        for (int i = 0; i < n; i++)
        {
            if(WiFi.SSID(i) == s->wifiOverrideESSID)
            {
                currentWifiMode = wifiOverride;
                Serial.print("Overmind found! joining : ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" - ");
                Serial.println(s->wifiOverridePasswd);
                WiFi.softAPdisconnect(true);
                yield();
                WiFi.mode(WIFI_STA);
                WiFi.begin(s->wifiOverrideESSID, s->wifiOverridePasswd);
                yield();
                overmindFound = true;
    }}}

  if(overmindFound)
  {
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }
  else if(s->wifiMode == wifiAP)
  {
      Serial.print("AP_MODE:");
      Serial.print(s->wifiESSID);
      Serial.print(" - ");
      Serial.println(s->wifiPasswd);
      WiFi.disconnect();
      yield();
      WiFi.softAP(s->wifiESSID, s->wifiPasswd);
      wifiAPMode = true;
      delay(2000);
  } else if(s->wifiMode == wifiClient) {
      Serial.print("WIFI_CLIENT_MODE:");
      Serial.print(s->wifiESSID);
      Serial.print(" - ");
      Serial.println(s->wifiPasswd);
      WiFi.softAPdisconnect(true);
      yield();
      if(s->staticIP)
      {
          IPAddress ip     (s->IP[0]    ,s->IP[1]    ,s->IP[2]    ,s->IP[3]);
          IPAddress gateway(0          ,0          ,0          , 0);
          IPAddress subnet (s->subnet[0],s->subnet[1],s->subnet[2],s->subnet[3]);
          WiFi.config      (ip, gateway, subnet);
      }
      WiFi.mode(WIFI_STA);
      WiFi.begin(s->wifiESSID, s->wifiPasswd);
      yield();
      uint8_t i = 0;
      while ( (WiFi.status() != WL_CONNECTED) && (i < 20) )
      {
        delay(500);
        Serial.print(".");
        i++;
      }
      if(WiFi.status() != WL_CONNECTED)
          Serial.println("Not Connected!");
      else
          Serial.println("Connected!");
  } else if(s->wifiMode == wifiMesh) {

  }

    IPAddress ip;
    String localIP;
    if(wifiAPMode)
        ip = WiFi.softAPIP();
    else
        ip = WiFi.localIP();
    localIP += ip[0];
    localIP += ".";
    localIP += ip[1];
    localIP += ".";
    localIP += ip[2];
    localIP += ".";
    localIP += ip[3];

    if(s->MdnsEnabled)
    {
        if(MDNS.begin(s->id))
        {
            Serial.print("MDNS started, name:");Serial.print(s->id);Serial.println(".local");
            localIP = "";
            localIP += s->id;
            localIP += ".local";
        }
        else
        {
            Serial.println("Failed to start MDNS");
        }
    }

    if(s->httpUpdaterEnabled)
    {
        MDNS.addService("http", "tcp", s->httpUpdatePort);
        startHttpUpdater(s->httpUpdatePort);
        Serial.print("Updater ready, Open http://");
        Serial.print(localIP);
        Serial.print(":");
        Serial.print(s->httpUpdatePort);
        Serial.println(" to upload hex");
    }

    if(s->httpServerEnabled)
    {
        MDNS.addService("http", "tcp", s->httpPort);
        startHttpFileServer(s->httpPort);
        Serial.print("HttpServer ready, Open http://");
        Serial.print(localIP);
        Serial.print(":");
        Serial.println(s->httpPort);
    }
}

#endif // WEBHELPERS_H
