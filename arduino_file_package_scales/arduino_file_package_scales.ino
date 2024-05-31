#include <WiFi.h>
#include <Arduino.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "ARNUR";
const char* password = "takonmama";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// variable initiation
// Deklarasi variabel tipe long untuk menyimpan durasi waktu dan mili detik
long duration, currentmillis, previousmillis, previousmillis2, interval = 2000, interval2 = 5000;

// Deklarasi variabel tipe float untuk menyimpan data yang mengandung pecahan desimal
float distanceCm, weight, volume, panjang, lebar, tinggi;

void setup() {
  Serial.begin(115200); // Starts the serial communication

  setup_wifi();
  initLittleFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getDataReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {  
  currentmillis = millis();
  if(currentmillis - previousmillis >= interval){
    previousmillis = currentmillis;
    volume = measureVolume();
    weight = measureWeight();

    // Send data to the client
    events.send("ping",NULL,millis());
    events.send(getDataReadings().c_str(),"new_readings" ,millis());

    Serial.println("Publishing data...");
    Serial.print("Volume");
    Serial.println(volume);
    Serial.print("Berat");
    Serial.println(weight);
  }
}

// Fungsi untuk mengukur berat secara acak
float measureWeight(){
  // Menghasilkan nilai acak antara 1000 dan 3000
  return random(1, 20); // Ubah rentang sesuai kebutuhan
}

// Fungsi untuk mengukur volume secara acak
float measureVolume(){
  // Menghasilkan nilai acak antara 500 dan 1500
  return random(500, 12500); // Ubah rentang sesuai kebutuhan
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Initialize LittleFS
void initLittleFS() {
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("LittleFS mounted successfully");
}

// Get Data Readings and return JSON object
String getDataReadings(){
  readings["weight"] = String(measureWeight());
  readings["volume"] = String(measureVolume());
  String jsonString = JSON.stringify(readings);
  return jsonString;
}
