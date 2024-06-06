#include <Arduino.h>
#include <WiFi.h>               // Library Wifi
#include "LittleFS.h"           // Library file sistem ESP32 reference https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/ 
#include <AsyncTCP.h>           // Library untuk Asinkron TCP
#include <ESPAsyncWebServer.h>  // Library ESP Web Seb Server Asinkron
#include <Arduino_JSON.h>       // Library JSON (JavaScript Object National) arduino
#include <LiquidCrystal_I2C.h>  // Library LCD I2C
#include "HX711.h"              // Library Loadcell HX711

#define CALIBRATION_FACTOR 98   // Kalibrasi faktor Loadcell

// define pinout used
#define LOADCELL_DT_PIN 18      // Pin 18 to Pin DT HX711 Module
#define LOADCELL_SCK_PIN 19     // Pin 19 to Pin SCK HX711 Module
#define SPARE_BUTTON_PIN 4      // Pin 4 to Push Button Tare

// Replace the next variables with your SSID/Password combination
const char* ssid = "Redmi 7A";
const char* password = "asdfghjkl";

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// 0 atas// 1 kanan// 2 depan// 3 kiri// 4 belakang
const int trigPin[] = {13,14,26,33,15}; // Pin ESP32 for trig pin HC-SR04 (ultrasonic module)
const int echoPin[] = {12,27,25,32,5};  // Pin ESP32 for echo ppin HC-SR04 (ultrasonic module)
float distanceArr[5];
float distanceLimit[] = {50.5,24.15,23.9,24.2,23.9}; // Naikan nilainya jika pembacaan < ukuran real atau turunkan nilainya jika pembacaan > ukuran real  
LiquidCrystal_I2C lcd(0x27, 20, 4); // Use lcd i2c 20x4
HX711 scale;

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

// Deklarasi variabel tipe int untuk menyimpan angka bulat
int a;

// Deklarasi array char untuk menyimpan karakter
char buffer[20]; // Buffer karakter dengan panjang 20

// Pendefinisian fungsi
float measureDistance(int i);
float measureVolume();
float measureWeight();
void displayWelcome();
void displayData(float volume, float weight);
void displayReady();
void displayTare(); // Fungsi untuk meng nol kan pembacaan berat
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
void initLittleFS();
String getDataReadings();

void setup() {
  Serial.begin(115200); // Starts the serial communication

  for(int i=0;i<5;i++){
    pinMode(trigPin[i], OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin[i], INPUT); // Sets the echoPin as an Input
  }

  pinMode(SPARE_BUTTON_PIN, INPUT_PULLUP);
  // LCD setup
  lcd.init(); // initialize LCD
  lcd.backlight(); // turn on LCD backlight
  displayWelcome();
  // loadcell setup
  // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  scale.begin(LOADCELL_DT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare(); // reset the scale to 0

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

    if(weight < 0.01){
      displayReady();
      Serial.println("Siap Mengukur");
    } else {
      displayData(volume, weight);
      Serial.print("Volume: ");
      Serial.println(volume);
      Serial.print("Berat: ");
      Serial.println(weight);
    }

    // Send data to the client
    events.send("ping",NULL,millis());
    events.send(getDataReadings().c_str(),"new_readings" ,millis());

    Serial.println("Publishing data...");

    if(digitalRead(SPARE_BUTTON_PIN) == LOW){
      displayTare();
      delay(100);
    }
  }
}

float measureDistance(int i){
  // Clears the trigPin
  digitalWrite(trigPin[i], LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin[i], HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin[i], LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin[i], HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  // Prints the distance in the Serial Monitor
  Serial.print("Channel ");
  Serial.print(i);
  Serial.print(" Distance (cm): ");
  Serial.println(distanceCm);
  // delay(1000);
  return distanceCm;
}

float measureWeight(){
  return scale.get_units(10)/1000; // dibagi 1000 untuk menghitung kilogram
}

float measureVolume(){
  //float panjang;
  //float lebar;
  //float tinggi;
  float volume;
  for(int i=0; i<5; i++){
    distanceArr[i] = measureDistance(i);
  }
  panjang = ((distanceLimit[1] - distanceArr[1]) + (distanceLimit[3] -
  distanceArr[3]));
  lebar = ((distanceLimit[2] - distanceArr[2]) + (distanceLimit[4] -
  distanceArr[4]));
  tinggi = distanceLimit[0] - distanceArr[0];
  volume = (panjang * lebar * tinggi);
  Serial.print("panjang: ");
  Serial.print(panjang);
  Serial.print(" lebar: ");
  Serial.print(lebar);
  Serial.print(" tinggi: ");
  Serial.print(tinggi);
  Serial.print(" volume: ");
  Serial.println(volume);
  // if(distanceArr[0] < distanceLimit[0]){
  // return 0;
  // }else{
  // return volume;
  // }
  return volume;
}

void displayData(float volume, float weight){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P:");
  lcd.print(panjang);
  lcd.setCursor(7,0);
  lcd.print("cm");
  lcd.setCursor(10, 0);
  lcd.print("L:");
  lcd.print(lebar);
  lcd.setCursor(18,0);
  lcd.print("cm");
  lcd.setCursor(5, 1);
  lcd.print("T : ");
  lcd.print(tinggi);
  lcd.setCursor(13,1);
  lcd.print("cm");
  lcd.setCursor(0, 2);
  lcd.print("Volume: ");
  lcd.print(volume);
  lcd.setCursor(16,2);
  lcd.print("cm3");
  lcd.setCursor(0, 3);
  lcd.print("Berat: ");
  lcd.print(weight);
  lcd.setCursor(15,3);
  lcd.print("Kg");
}

void displayWelcome(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALAT PENGUKUR BERAT");
  lcd.setCursor(8, 1);
  lcd.print("dan");
  lcd.setCursor(4, 2);
  lcd.print("DIMENSI PAKET");
  delay(3000);

}

void displayReady() {
  lcd.clear();
  lcd.setCursor(2,1);
  lcd.print("ALAT SIAP UNTUK");
  lcd.setCursor(6,2);
  lcd.print("MENGUKUR");
  delay(2000);
}

void displayTare(){
  scale.tare(); // Fungsi untuk meng nol kan pembacaan loadcell (builtin library HX711)

  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("KALIBRASI SENSOR");
  lcd.setCursor(2, 2);
  lcd.print("BERAT");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("BERAT SAAT INI:");
  lcd.setCursor(2, 2);
  lcd.print(scale.get_units(10)/1000); // Untuk mendapatkan nilaiberat (KG) saat ini
  lcd.print(" Kg");
  delay(2000);
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

  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("IP Address: ");
  lcd.setCursor(1, 2);
  lcd.print(WiFi.localIP());
  delay(10000); // lama menampilkan alamat IP pada lcd 10 detik
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
