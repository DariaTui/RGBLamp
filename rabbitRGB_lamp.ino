#include <ESP8266WiFi.h>                 
#include <ESP8266WebServer.h>           
#include <ESP8266SSDP.h>                 
#include <FS.h>                         

ESP8266WebServer HTTP(80);              

String AP       = "LoginFromWifi";      
String PASSWORD = "*******";     

String SSDP_Name = "RabbitGB_Lamp";         

#define r 2                             
#define g 0                             
#define b 3                             
#define button 1                       

int ledmode = 1;                        // Переменная для изменения режимов работы светильника
int cr,cg,cb;                           // Переменные для хранения текущих значений цветов (от 0 до 255 для каждого цвета)

int active_color;                       // переменная для перебора цветов
int i;                                  // переменная для плавной смены цветов
boolean turn = true;                    // логическая переменная для определения направления изменения спектра (true к 255, false к 0)

void setup() {
  pinMode(r,OUTPUT);                        
  pinMode(g,OUTPUT);                        
  pinMode(b,OUTPUT);                        
  pinMode(button,INPUT);                    

  analogWrite(r,invert(255));               
  analogWrite(g,invert(0));                 
  analogWrite(b,invert(0));                 

  active_color = g;                         // программа начнет работу зеленого цвета
  i = 0;                                    

  WiFi.mode(WIFI_STA);                      // Определяем режим работы Wi-Fi модуля в режиме клиента
  WiFi.begin(AP.c_str(), PASSWORD.c_str()); 
  HTTP.begin();                             // Инициализация Web-сервер
  SPIFFS.begin();                           // Инициализация файловой системы
  SSDP_init();                               

// обработка HTTP-события 
  HTTP.on("/rainbow", [](){                   
    ledmode = 1;                              
    HTTP.send(200, "text/plain", "rainbow");  
  });
  HTTP.on("/red", [](){                       
    ledmode = 2;                              
    HTTP.send(200, "text/plain", "red");      
  });  
  HTTP.on("/orange", [](){                    
    ledmode = 3;                              
    HTTP.send(200, "text/plain", "orange");   
  });
  HTTP.on("/yellow", [](){                    
    ledmode = 4;                              
    HTTP.send(200, "text/plain", "yellow");   
  });
  HTTP.on("/green", [](){                     
    ledmode = 5;                              
    HTTP.send(200, "text/plain", "green");    
  });
  HTTP.on("/lightblue", [](){                 
    ledmode = 6;                              
    HTTP.send(200, "text/plain", "lightblue");
  });
  HTTP.on("/blue", [](){                      
    ledmode = 7;                              
    HTTP.send(200, "text/plain", "blue");     
  });
  HTTP.on("/violet", [](){                    
    ledmode = 8;                              
    HTTP.send(200, "text/plain", "violet");   
  });
  HTTP.on("/white", [](){                     
    ledmode = 9;                              
    HTTP.send(200, "text/plain", "white");    
  });
  //обработка ошибки на сервере
  HTTP.onNotFound([](){                                   
    if(!handleFileRead(HTTP.uri()))                       
      HTTP.send(404, "text/plain", "File isn't found");   
  });
}

void loop() {
  HTTP.handleClient();                        // применение алгоритма для обработки HTTP события
  if (digitalRead(button) == HIGH && debounce()) {  //работа с кнопкой и вызова функции для предотвращения дребезга
   ledmode++;                                       
   if (ledmode > 9)                           
     ledmode = 1;                             
  }
  if (ledmode == 9)                           
    smooth_white();                           
  else                                        
    smooth_change();                          
  delay(5);                                  
}
  // Функция работы с файловой системой
bool handleFileRead(String path){                     
  if(path.endsWith("/")) path += "index.htm";           
  String contentType = getContentType(path);            
  String pathWithGz = path + ".gz";                     
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){ 
    if(SPIFFS.exists(pathWithGz))                       
      path += ".gz";                                    
    File file = SPIFFS.open(path, "r");                 
    size_t sent = HTTP.streamFile(file, contentType);   
    file.close();                                       
    return true;                                        
  }
  return false;                                         
}

String getContentType(String filename){                                 
  if (HTTP.hasArg("download")) return "application/octet-stream";       
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
// Функция создает описание устройства для его обнаружения в сетевом окружении
void SSDP_init(void) {                                                  
  HTTP.on("/description.xml", HTTP_GET, []() {                       
    SSDP.schema(HTTP.client());                                        
  });
  SSDP.setDeviceType("upnp:rootdevice");                             
  SSDP.setSchemaURL("description.xml");                               
  SSDP.setHTTPPort(80);                                               
  SSDP.setName(SSDP_Name);                                             
  SSDP.setSerialNumber(ESP.getChipId());                            
  SSDP.setURL("/");                                         
  SSDP.setModelName(SSDP_Name);                                      
  SSDP.begin();                                                         
}
// Функция преобразования значений
int invert(int value){                  
  return map(value, 0, 255, 1023, 0);  
}
// Функция для самостоятельной смены цветов диода
void change_color() {   
  switch (active_color) {               
    case r:                             
      active_color = b;                 
      return;                           
    case g:                             
      active_color = r;                 
      return;                           
    case b:                             
      active_color = g;                 
      return;                           
  }
}
// Функция для включения белого цвета
void smooth_white() {                   
    if (cr == 255 && cg == 255 && cb == 255)  
      return;                                 
    if (cr < 255)                             
      cr++;                                   
    if (cg < 255)                             
      cg++;                                   
    if (cb < 255)                             
      cb++;                                   
    analogWrite(r,invert(cr));                
    analogWrite(g,invert(cg));                
    analogWrite(b,invert(cb));                
}
// Функция для смены цвета
void smooth_change() {                  

  if (ledmode == 2 && cr == 255 &&  cg == 0 && cb == 0)     
    return;                                                 

  if (ledmode == 3 && cr == 255 &&  cg == 70 && cb == 0)    
    return;                                                 

  if (ledmode == 4 && cr == 255 &&  cg == 255 && cb == 0)   
    return;                                                 

  if (ledmode == 5 && cr == 147 &&  cg == 255 && cb == 0)   
    return;                                                 

  if (ledmode == 6 && cr == 0 &&  cg == 255 && cb == 200)  
    return;                                                 

  if (ledmode == 7 && cr == 0 &&  cg == 0 && cb == 255)    
    return;                                                 

  if (ledmode == 8 && cr == 255 &&  cg == 0 && cb == 255)   
    return;                                                

  if (cr != 0 &&  cg != 0 && cb != 0) {                   
    cg--;                                                  
    cb--;                                                   
    analogWrite(g,invert(cg));                              
    analogWrite(b,invert(cb));                             
    turn = true;                                            
    i = 0;                                                 
    active_color = g;                            
    return;                                         
  }
  
  if (turn)                                             
    i++;                                                
  else                                                    
    i--;                                                 

  analogWrite(active_color,invert(i));      
// Функция чтобы запомнить текущие значения на всех цветовых каналах
  mem_current_colors();                     

  if ((i == 255 && turn) || (i == 0 && !turn)) { 
    change_color();                               
    turn = !turn;                                
  }
}
 // Функция для запоминания текущих значение цветов
void mem_current_colors() {                 
  switch (active_color) {                   
    case r:                                 
      cr = i;                               
      return;                              
    case g:                              
      cg = i;                              
      return;                             
    case b:                                
      cb = i;                       
      return;                             
  }  
}
//Функция для обработки дребезга
bool debounce() {                         
  int high_level = 0;                      
  for (int n = 0; n < 100; n++) {           
    if (digitalRead(button) == HIGH)       
      high_level++;                         
  }
  if (high_level > 70) {                   
     delay(1000);            
     return true;                         
  }
  return false;                
}

