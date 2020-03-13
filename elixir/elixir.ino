//=======================================================================
//                     WiFi/SoftAP SETUP
//=======================================================================

#include <ESP8266WiFi.h>
#define WIFI_SSID "<YOUR SSID>"
#define WIFI_PASSWORD "<YOUR PASSWORD>"
#define WIFI_LED BUILTIN_LED
IPAddress local_ip(192,168,0,200);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(192,169,0,1);

#include <ESP8266Ping.h>
const char* remote_host = "www.google.com";

//=======================================================================
//                     WebServer SETUP
//=======================================================================

#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

//=======================================================================
//                     SPIFFS SETUP
//=======================================================================

#include <FS.h>
#include <ArduinoJson.h>

//=======================================================================
//                     OTA SETUP
//=======================================================================

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
bool ota_flag = false;
uint16_t time_elapsed = 0;

//=======================================================================
//                     FIREBASE SETUP
//=======================================================================

#include <FirebaseArduino.h>
#define FIREBASE_HOST "<YOUR FIREBASE PROJECT URL>"
#define FIREBASE_AUTH "<YOUR FIREBASE AUTH KEY>"

//=======================================================================
//                     TANK SETUP
//=======================================================================

String supply_tank = "";
String storage_tank = "";
bool flag = true;
bool manual = false;
bool relay = false;

//=======================================================================
//                     SETUP
//=======================================================================

void setup() {
  
  Serial.begin(9600);
  SPIFFS.begin();   // Initiating SPIFFS
  
  pinMode(D0, OUTPUT);
  
  WiFi.hostname("elixir");
//  WiFi.config(local_ip, gateway, subnet, dns);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.localIP());

  // Initializing OTA
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // Server Client Request Handlers Setup
  server.on("/", handle_on_start);
  server.on("/relay_on", relay_on);
  server.on("/relay_off", relay_off);
  server.onNotFound(handle_NotFound);

  // Starting ESP server
  if (MDNS.begin("elixir")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  server.begin();
  Serial.println("Starting local server...");
  Serial.println("Initial Setup at " + WiFi.localIP());
  delay(1000);

  // Supply Tank
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  
  // Storage Tank
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);

  // Relay
  pinMode(D3, OUTPUT);
  digitalWrite(D3, HIGH);
}

//=======================================================================
//                        LOOP
//=======================================================================

void loop() {

  if(ota_flag)
  {
    uint16_t time_start = millis();
    Serial.println("Listening for OTA...");
    while(time_elapsed < 30000)
    {
      ArduinoOTA.handle();
      time_elapsed = millis()-time_start;
      delay(10);
      digitalWrite(2, !digitalRead(2));
      delay(100);
    }
    ota_flag = false;
  }

  if(!manual) {

    if(digitalRead(D7)) {
      supply_tank = "E";
    } else if(digitalRead(D6)) {
      supply_tank = "L";
    } else if(digitalRead(D5)) {
      supply_tank = "M";
    } else {
      supply_tank = "H";
    }

    if(digitalRead(D2)) {
      storage_tank = "E";
      digitalWrite(D3, HIGH);
      relay = false;
    } else if(digitalRead(D1)) {
      storage_tank = "A";
    } else {
      storage_tank = "F";
    }

    if(supply_tank == "E") {
      if((storage_tank == "A") || (storage_tank == "F")) {
        digitalWrite(D3, LOW);
        relay = true;
      }
    }

    if(supply_tank == "H") {
      digitalWrite(D3, HIGH);
      relay = false;
    }
  }
  
  server.handleClient();
}


// Server Request Handler Functions
void handle_on_start() {
  String addr = "/" + storage_tank + "/" + supply_tank + "/" + String(relay, BIN);
//  server.sendHeader("location", addr);
  server.send(200, "text/plane", addr); 
}

void relay_on() {
  manual = true;
  digitalWrite(D3, LOW);
  relay = true;
  Serial.println("Relay HIGH!");
  server.sendHeader("location", "/");
  server.send(302, "text/plane");
}

void relay_off() {
  digitalWrite(D3, HIGH);
  relay = false;
  Serial.println("Relay LOW!");
  server.sendHeader("location", "/");
  server.send(302, "text/plane");
  manual = false;
}

void handle_NotFound(){
  server.sendHeader("Location", "/", true);
  server.send(404, "text/plain", "Not found");
}#include <ESP8266WiFi.h>
#define WIFI_SSID "CREED's Link"
#define WIFI_PASSWORD "pa55w0rd@"
#define WIFI_LED BUILTIN_LED

#include <FirebaseArduino.h>
#define FIREBASE_HOST "elixir-c708f.firebaseio.com"
#define FIREBASE_AUTH "1zCSUoq3PVaTEwFLjJLWHarHU2s2jC0hvfAtVMEi"

// Defining Relay Switch Pin
#define relay_switch D1

// Defining Senson Pins
// Supply Tank
#define supply_low D2
#define supply_med D3
#define supply_high D5

int supply_level = 0;

// Storage Tank
#define storage_low D6
#define storage_med D7
#define storage_high D8

int storage_level = 0;

void setup() {

  pinMode(relay_switch, OUTPUT);
  pinMode(supply_low, INPUT);
  pinMode(supply_med, INPUT);
  pinMode(supply_high, INPUT);
  pinMode(storage_low, INPUT);
  pinMode(storage_med, INPUT);
  pinMode(storage_high, INPUT);
  
  Serial.begin(9600);
  pinMode(WIFI_LED, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST);
  Firebase.stream("/relay");
}

void loop() {

  if(digitalRead(storage_low) == HIGH) {
    Firebase.setInt("/storage/low", 0);
  } else if(digitalRead(storage{
    Firebase.setInt("/storage/low", 1);
  }
  
  if(Firebase.failed()) {
    Serial.print("Setting number failed: ");
    Serial.println(Firebase.error());
    return;
  }
 
  if (Firebase.available()) {
     FirebaseObject event = Firebase.readEvent();
     String eventType = event.getString("type");
     eventType.toLowerCase();
     
     Serial.print("event: ");
     Serial.println(eventType);
     if (eventType == "put") {
       String path = event.getString("path");
       int data = event.getInt("data");
       Serial.print(path + " : ");
       Serial.println(data);
     }
  }
}
