#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "RTClib.h"
#include "LittleFS.h"
#include <Adafruit_NeoPixel.h>

const char* ssid_AP_F = "LocalCO2-01";
const char* pasw_AP_F = "123456789";

//int    wifi_MOD_Current = 0;
int    wifi_MOD = 0; //0 = AP, 1 = STA, 2 = Default AP

String ssid_AP = "LocalCO2-U";
String pasw_AP = "12341234";
String ssid_STA = "ZeroAP";
String pasw_STA = "zero9966";

bool networkChange = false;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
 
//PARAMS
int MIN_WRITE = 1;
int RANGE_GOOD = 600;
int RANGE_UNHEALTHY = 1000;

//Lecturas validas
int mea_current =  0;
int mea_write = 0;
int seq = 1;
int seqmax = 4;

int circleratio = 3;
int circlesep   = 9;   
float CO2 = 0;
String CO2Measurement = "";

SCD30 airSensor;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
RTC_DS3231 rtc;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "f";

const char* PARAM_INPUT_rg = "rg";
const char* PARAM_INPUT_ru = "ru";
const char* PARAM_INPUT_wi = "wi";

const char* PARAM_INPUT_md = "mod";
const char* PARAM_INPUT_ap_ssid = "aps";
const char* PARAM_INPUT_ap_pasw = "app";
const char* PARAM_INPUT_st_ssid = "sts";
const char* PARAM_INPUT_st_pasw = "stp";

const char* PARAM_INPUT_crf = "crf";
int CALIBRATE_REF = 400;
bool calibrateInit = false;
  
#define LEDS_PIN 14
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, LEDS_PIN, NEO_GRB + NEO_KHZ800);

bool resetCSVFile = false;
String resetCSVFileName = "";

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  
  Serial.println("SCD30 Example");
  Wire.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1);
  }
  //airSensor.setMeasurementInterval(2); //Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
  //While the setting is recorded, it is not immediately available to be read.
  delay(200);

  int interval = airSensor.getMeasurementInterval();
  Serial.print("Measurement Interval: ");
  Serial.println(interval);

  //airSensor.setAltitudeCompensation(1600); //Set altitude of the sensor in m, stored in non-volatile memory of SCD30

  //Read altitude compensation value
  unsigned int altitude = airSensor.getAltitudeCompensation();
  Serial.print("Current altitude: ");
  Serial.print(altitude);
  Serial.println("m");

  //Pressure in Boulder, CO is 24.65inHg or 834.74mBar
  //airSensor.setAmbientPressure(835); //Current ambient pressure in mBar: 700 to 1200, will overwrite altitude compensation
  //airSensor.setTemperatureOffset(5); //Optionally we can set temperature offset to 5°C, stored in non-volatile memory of SCD30

  //Read temperature offset
  float offset = airSensor.getTemperatureOffset();
  Serial.print("Current temp offset: ");
  Serial.print(offset, 2);
  Serial.println("C");
 
  display.display();
  delay(500); // Pause for half second

  // Clear the buffer
  display.clearDisplay();
 
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  
  //if (! rtc.isrunning()) {
    //Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  //}
  
  initLittleFS();

  initLedPixels();
  
  readParamsFile("/params.txt");

  readNetworkFile("/network.txt");

  initWifiMode();
  
  initWebserver();

  //
  mea_current = 0;
  mea_write = 30 * MIN_WRITE;
  seq = 1;
}
/*
void onWiFiEventModeChange(const WiFiEventModeChange& event){

}

void onWifiSTAConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi sucessfully.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void onWifiSTADisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi, trying to connect...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
} 

void onWifiAPConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi sucessfully.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void onWifiAPDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi, trying to connect...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
}
*/

void initLedPixels(){
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  //pixels.setBrightness(127); //50%
  pixels.setBrightness(64);    //25%
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();   // Send the updated pixel colors to the hardware.
}
 
void initWifiMode(){
  if(wifi_MOD == 0){
    connectWiFi_AP();
  } else if(wifi_MOD == 1){
    connectWiFi_STA();
  } else if(wifi_MOD == 2){
    connectWiFi_AP_F();
  }
}

void connectWiFi_AP()
{ 
  Serial.println("");
  WiFi.mode(WIFI_AP);
  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.setCursor( 8,12);
  display.print("Red: ");
  display.print(ssid_AP);
  display.display();

  display.setCursor( 8,28); 
  int wifi_retry = 1;
  while(!WiFi.softAP(ssid_AP, pasw_AP))
  {
    Serial.println(".");
    display.print(".");
    display.display();
    for(int i=0; i<wifi_retry; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(0, 25, 255));
    }
    pixels.show();
    wifi_retry++;
    delay(200);
  }
  
  display.setCursor( 8,44); 
  display.print(WiFi.softAPIP());
  display.display();
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 228, 255));
  }
  pixels.show();
  delay(2000);
  
  Serial.println("");
  Serial.print("Iniciado AP:\t");
  Serial.println(ssid_AP);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  //wifi_MOD_Current = 0;
}

void disconnectWiFi_AP()
{ 
  Serial.print("Desconectando AP:\t");
  Serial.println(ssid_AP);
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay(100);
}

void connectWiFi_AP_F()
{ 
  Serial.println("");
  WiFi.mode(WIFI_AP);
  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.setCursor( 8,12);
  display.print("Red: ");
  display.print(ssid_AP_F);
  display.display();

  display.setCursor( 8,28); 
  int wifi_retry = 1;
  while(!WiFi.softAP(ssid_AP_F, pasw_AP_F))
  {
    Serial.println(".");
    display.print(".");
    display.display();
    for(int i=0; i<wifi_retry; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(0, 25, 255));
    }
    pixels.show();
    wifi_retry++;
    delay(200);
  }

  display.setCursor( 8,44); 
  display.print(WiFi.softAPIP());
  display.display();
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 228, 255));
  }
  pixels.show();
  delay(2000);
  
  Serial.println("");
  Serial.print("Iniciado AP:\t");
  Serial.println(ssid_AP_F);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
}

void connectWiFi_STA()
{
  //Register event handlers
  //wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  //wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_STA, pasw_STA);

  display.clearDisplay();
  display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.setCursor( 8,12);
  display.print("Red: ");
  display.print(ssid_STA);
  display.display();
  
  display.setCursor( 8,28); 
  int wifi_retry = 1;
  while (WiFi.status() != WL_CONNECTED) 
  { 
    if(wifi_retry == 6){
      //change mode and restart
      updateNetwork("2", ssid_AP, pasw_AP, ssid_STA, pasw_STA);
      delay(500);
      //reset cambio de configuración de red
      ESP.reset();
      delay(1000);
    }
    display.print(".");
    display.display();
    for(int i=0; i<wifi_retry; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(0, 25, 255));
    }
    pixels.show();
    wifi_retry++;
    delay(2000);
  }
  display.setCursor( 8,44); 
  display.print(WiFi.localIP());
  display.display();
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 228, 255));
  }
  pixels.show();
  delay(2000);
  
  Serial.println("");
  Serial.print("Iniciado STA:\t");
  Serial.println(ssid_STA);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  //WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);
}
 
void initWebserver(){
  
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  });  
  // Route for resources files
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/x-icon");
  });
  server.on("/pico.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/pico.min.css", "text/css");
  });
  server.on("/jquery-3.6.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/jquery-3.6.1.min.js", "application/javascript");
  }); 
  server.on("/modal.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/modal.min.js", "application/javascript");
  });  
  server.on("/chart.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/chart.min.js", "application/javascript");
  });
  server.on("/luxon.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/luxon.min.js", "application/javascript");
  }); 
  server.on("/chartjs-adp-luxon.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/chartjs-adp-luxon.js", "application/javascript");
  });
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.js", "application/javascript");
  });
  //server.on("/values.csv", HTTP_GET, [](AsyncWebServerRequest *request) {
  //  request->send(LittleFS, "values.csv", "text/plain");
  //});
  server.on("/values", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      String f = request->getParam(PARAM_INPUT_1)->value();
      request->send(LittleFS, "csv/" + f, "text/plain");
    }
  });
  server.on("/file", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      String f = request->getParam(PARAM_INPUT_1)->value();
      request->send(LittleFS, "csv/" + f, "text/plain", true);
    }
    //request->send(LittleFS, "values.csv", "text/plain", true);
  });
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_txt = "set reset csv";
    resetCSVFile = true;
    //request->send(200, "text/plain", response_txt);
    if (request->hasParam(PARAM_INPUT_1)) {
      String f = request->getParam(PARAM_INPUT_1)->value();
      resetCSVFileName = f;
      request->send(200, "text/plain", response_txt);
    }
  });
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_txt = displayCSVFiles();
    request->send(200, "text/plain", response_txt);
  });
  server.on("/params", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_txt = "set params";
    if (request->hasParam(PARAM_INPUT_rg)) {
      String rg = request->getParam(PARAM_INPUT_rg)->value();
      String ru = request->getParam(PARAM_INPUT_ru)->value();
      String wi = request->getParam(PARAM_INPUT_wi)->value();
      response_txt = updateParams(rg, ru, wi);
      request->send(200, "text/plain", response_txt);
    }
  });
  server.on("/paramsfile", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "params.txt", "text/plain", true);
  });
  server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_txt = "set network";
    if (request->hasParam(PARAM_INPUT_md)) {
      String mod = request->getParam(PARAM_INPUT_md)->value();
      String aps = request->getParam(PARAM_INPUT_ap_ssid)->value();
      String app = request->getParam(PARAM_INPUT_ap_pasw)->value();
      String sts = request->getParam(PARAM_INPUT_st_ssid)->value();
      String stp = request->getParam(PARAM_INPUT_st_pasw)->value();
      response_txt = updateNetwork(mod, aps, app, sts, stp);
      request->send(200, "text/plain", response_txt);
    }
  });
  server.on("/networkfile", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "network.txt", "text/plain", true);
  });
  server.on("/calibrate", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response_txt = "init calibrate";
    if (request->hasParam(PARAM_INPUT_crf)) {
      String crf = request->getParam(PARAM_INPUT_crf)->value();
      response_txt = initCalibrate(crf);
      request->send(200, "text/plain", response_txt);
    }
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(400, "text/plain", "Not found");
  });
  // Start server
  server.begin();
}

void initLittleFS(){ 
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
}

void closeLittleFS(){ 
  LittleFS.end();
}
/* 
void writeData(String data)
{
  //Open the file
  File file = LittleFS.open("/values.txt", "a");
  //Write to the file
  file.print(data);
  delay(1);
  //Close the file
  file.close();
  Serial.println("Write successful");
}
*/

void readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.print("Read from file: ");
  while (file.available()) { Serial.write(file.read()); }
  file.close();
}

void writeFile(const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000);  // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path); 
  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteFile(const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void replaceFile(const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path); 
  File file;
  //If not exists we will created
  if(!LittleFS.exists(path))
  {
    file = LittleFS.open(path, "w+");
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
    if (file.print(message)) {
      Serial.println("File created and message appended");
    } else {
      Serial.println("File created, append failed");
    }
  } else {
    file = LittleFS.open(path, "w");
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void updateFile(const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path); 
  File file;
  //If not exists we will created
  if(!LittleFS.exists(path))
  {
    file = LittleFS.open(path, "a+");
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
    if (file.print("time,co2")) {
      Serial.println("File created and message appended");
    } else {
      Serial.println("File created, append failed");
    }
  } else {
    file = LittleFS.open(path, "a");
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void displayDirFiles(){
  // Open dir folder
  Dir dir = LittleFS.openDir("/csv");
  // Cycle all the content
  while (dir.next()) {
    // get filename
    Serial.print(dir.fileName());
    Serial.print(" - ");
    // If element have a size display It else write 0
    if(dir.fileSize()) {
      File f = dir.openFile("r");
      Serial.println(f.size());
      f.close();
    }else{
      Serial.println("0");
    }
  }    
}

void readParamsFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  String line;
  int i = 0;
  char buffer[64];
  Serial.print("Read from file: ");
  while (file.available()) { 
    int l = file.readBytesUntil('\n', buffer, sizeof(buffer));
    buffer[l] = 0;
    if (i == 0) {
      line = buffer;
      RANGE_GOOD = line.substring(3).toInt();
      Serial.print("RANGE_GOOD:");
      Serial.println(RANGE_GOOD);
    }
    if (i == 1) {
      line = buffer;
      RANGE_UNHEALTHY = line.substring(3).toInt();
      Serial.print("RANGE_UNHEALTHY:");
      Serial.println(RANGE_UNHEALTHY);
    }
    if (i == 2) {
      line = buffer;
      MIN_WRITE = line.substring(3).toInt();
      Serial.print("MIN_WRITE:");
      Serial.println(MIN_WRITE);
    }
    i++;
    if (i == 3){
      break;
    }
    //Serial.write(file.read());
  }
  file.close();
}

String updateParams(const String& rg, const String& ru, const String& wi){
  // Open dir folder
  String params = "";
  RANGE_GOOD = rg.toInt();
  RANGE_UNHEALTHY = ru.toInt();
  MIN_WRITE = wi.toInt();
  replaceFile("/params.txt", ("rg:" + rg + "\n" + "ru:" + ru + "\n" + "wi:" + wi).c_str());
  //reset tiempo de escritura
  mea_current = 0;
  mea_write = 2 * 30 * MIN_WRITE;
  return params;    
}

void readNetworkFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);
  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    wifi_MOD = 2;
    return;
  }
  String line;
  int i = 0;
  char buffer[64];
  Serial.print("Read from file: ");
  while (file.available()) { 
    int l = file.readBytesUntil('\n', buffer, sizeof(buffer));
    buffer[l] = 0;
    if (i == 0) {
      line = buffer;
      wifi_MOD = line.substring(4).toInt();
      Serial.print("wifi_MOD:");
      Serial.println(wifi_MOD);
    }
    if (i == 1) {
      line = buffer;
      ssid_AP = line.substring(4);
      Serial.print("ssid_AP:");
      Serial.println(ssid_AP);
    }
    if (i == 2) {
      line = buffer;
      pasw_AP = line.substring(4);
      Serial.print("pasw_AP:");
      Serial.println(pasw_AP);
    }
    if (i == 3) {
      line = buffer;
      ssid_STA = line.substring(4);
      Serial.print("ssid_STA:");
      Serial.println(ssid_STA);
    }
    if (i == 4) {
      line = buffer;
      pasw_STA = line.substring(4);
      Serial.print("pasw_STA:");
      Serial.println(pasw_STA);
    }
    i++;
    if (i == 5){
      break;
    }
    //Serial.write(file.read());
  }
  file.close();
  //
  //wifi_MOD_Current = wifi_MOD;
}

String updateNetwork(const String& mod, const String& aps, const String& app, const String& sts, const String& stp){
  // Open dir folder
  String params = "";
  //RANGE_GOOD = rg.toInt();
  //RANGE_UNHEALTHY = ru.toInt();
  //MIN_WRITE = wi.toInt();
  
  wifi_MOD = mod.toInt();
  ssid_AP  = aps;
  pasw_AP  = app;
  ssid_STA = sts;
  pasw_STA = stp;

  //initWifiMode();

  replaceFile("/network.txt", ("mod:" + mod + "\n" + "aps:" + aps + "\n" + "app:" + app + "\n" + "sts:" + sts + "\n" + "stp:" + stp).c_str());
  //reset tiempo de escritura
 
  networkChange = true;
  
  return params;    
}

String displayCSVFiles(){
  // Open dir folder
  String fileNames = "";
  Dir dir = LittleFS.openDir("/csv");
  // Cycle all the content
  while (dir.next()) {
    // get filename
    Serial.println(dir.fileName()); 
    fileNames += dir.fileName() + ",";
  }
  return fileNames;    
}

String initCalibrate(const String& crf){
  String params = "";
  CALIBRATE_REF = crf.toInt();
  calibrateInit = true;
  return params;    
}

// Replaces placeholder with DHT values
String processor(const String& var){
  Serial.println(var);
  if(var == "SSID_AP_DFLT"){
    return String(ssid_AP_F);
  }
  else if(var == "PASW_AP_DFLT"){
    return String(pasw_AP_F);
  }
  return String();
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void loop()
{
  if (airSensor.dataAvailable())
  {
    CO2 = airSensor.getCO2();
    CO2Measurement = String(CO2, 0);
    
    Serial.print("co2(ppm):");
    Serial.println(CO2Measurement);
    if( CO2 > 0) 
    {
      display.clearDisplay();
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setTextColor(WHITE); 
      display.setCursor( 0,12);
      display.print(airSensor.getTemperature(), 0);
      display.setFont();
      display.setTextSize(1);
      display.print(" C");
    
      display.setFont(&FreeSans9pt7b);
      display.setTextSize(1);
      display.setTextColor(WHITE); 
      display.setCursor(96,12);
      display.print(airSensor.getHumidity(), 0);
      display.setFont();
      display.setTextSize(1);
      display.print(" %");
      
      //display.setFont(&FreeSans18pt7b);
      display.setFont(&FreeSans24pt7b);
      display.setTextSize(1);
      display.setTextColor(WHITE); 
      if(CO2 < 1000)   
        display.setCursor(26,60);
      else
        display.setCursor(12,60); 
      display.println(CO2Measurement);
 
      for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        if(CO2 < RANGE_GOOD)   
        {
          pixels.setPixelColor(i, pixels.Color(0, 150, 0));  
        }else if (CO2 < RANGE_UNHEALTHY){
          pixels.setPixelColor(i, pixels.Color(255,215, 0));  
        }else{
          pixels.setPixelColor(i, pixels.Color(150, 0, 0));  
        }
      }
      pixels.show();   // Send the updated pixel colors to the hardware.
      
      int circlenum   = seqmax;
      int16_t i;
      display.dim(false);
      for(i=1; i<=circlenum; i++) {
        if(i==seq){
          display.fillCircle(3, 24 + circlesep*i, circleratio, SSD1306_INVERSE);
        } else {
          display.drawCircle(3, 24 + circlesep*i, circleratio, SSD1306_INVERSE);
        }
      }

      display.display();
      display.clearDisplay();
      delay(100);
       
      if(seq==seqmax) {
        seq=1;
      }else{
        seq++;
      } 
      Serial.print("mea_current/mea_write: ");
      Serial.print(mea_current);
      Serial.print(" ");
      Serial.println(mea_write);
      mea_current++;
    }
    if(resetCSVFile==true)
    {
      deleteFile(("csv/" + resetCSVFileName).c_str());
      resetCSVFile=false;
      resetCSVFileName="";
    }
    
    if(mea_current==mea_write)
    {
      //
      DateTime nowdt = rtc.now(); 
      updateFile(("/csv/values-" + nowdt.timestamp(DateTime::TIMESTAMP_DATE) + ".csv").c_str(), ("\n" + nowdt.timestamp(DateTime::TIMESTAMP_FULL) + "," + CO2Measurement).c_str());
      //delay(5*60*1000);
      mea_current = 0;
    }  
    //
    if(calibrateInit==true){
      airSensor.setForcedRecalibrationFactor(CALIBRATE_REF);
      calibrateInit=false;
      for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
        pixels.setPixelColor(i, pixels.Color(0, 25, 255));
      }
      pixels.show();   // Send the updated pixel colors to the hardware.
    }
  }
  if(networkChange == true){
    //reset cambio de configuración de red
    ESP.reset();
    delay(1000);
  }
}
