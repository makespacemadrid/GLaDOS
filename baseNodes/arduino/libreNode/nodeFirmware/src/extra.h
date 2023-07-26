#ifndef AUXFUNC
#define AUXFUNC

String localIP()
{
  IPAddress ip;
//  if(Wifi.status() == 3)
    ip = WiFi.localIP();
//  else
//  ip = WiFi.softAPIP();

  String result;
  result += ip[0];
  result += ".";
  result += ip[1];
  result += ".";
  result += ip[2];
  result += ".";
  result += ip[3];
  return result;
}


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


#endif
