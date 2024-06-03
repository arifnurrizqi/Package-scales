#include <WiFi.h>
#include <Arduino.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

#define CALIBRATION_FACTOR 98
// define pinout used
#define LOADCELL_DT_PIN 18
#define LOADCELL_SCK_PIN 19
// #define RESET_BUTTON_PIN 2
#define SPARE_BUTTON_PIN 4

// Replace the next variables with your SSID/Password combination
const char* ssid = "Redmi 7A";
const char* password = "asdfghjkl";

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// 0 atas// 1 kanan// 2 depan// 3 kiri// 4 belakang
const int trigPin[] = {13,14,26,33,15};
const int echoPin[] = {12,27,25,32,5};
float distanceArr[5];
float distanceLimit[] = {47.12,22.645,22.745,22.55,22.28};
LiquidCrystal_I2C lcd(0x27, 20, 4); // Use lcd i2c 20x4
HX711 scale;
// WiFiClient espClient;

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
void displayReset();
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

  // pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
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
    displayData(volume, weight);

    // Send data to the client
    events.send("ping",NULL,millis());
    events.send(getDataReadings().c_str(),"new_readings" ,millis());

    Serial.println("Publishing data...");
    Serial.print("Volume: ");
    Serial.println(volume);
    Serial.print("Berat: ");
    Serial.println(weight);

    if(digitalRead(SPARE_BUTTON_PIN) == LOW){
      displayReset();
      scale.tare();
      while (digitalRead(SPARE_BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }

  if(Serial.available()){
    switch(Serial.read()) // akhiri setiap perintah dengan ';' contoh "s1000;"
    {
      case 's':
        a = Serial.parseInt();
        for(int i=0;i<5;i++){
          measureDistance(a);
        } 
        Serial.println();
        break;

      case 'a':
        Serial.println("test");
        break;

      case 'b':
        // set cursor to first column, first row
        lcd.setCursor(0, 0);
        // print message
        lcd.print("Hello, World!");
        delay(1000);
        // clears the display to print new message
        lcd.clear();
        // set cursor to first column, second row
        lcd.setCursor(0,1);
        lcd.print("Hello, World!");
        delay(1000);
        lcd.clear();
        break;

      case 'c':
        if (scale.is_ready()) {
          scale.set_scale();
          Serial.println("Tare... remove any weights from the scale.");
          delay(5000);
          scale.tare();
          Serial.println("Tare done...");
          Serial.print("Place a known weight on the scale...");
          delay(5000);
          long reading = scale.get_units(10);
          Serial.print("Result: ");
          Serial.println(reading);
        }
        else {
          Serial.println("HX711 not found.");
        }
        break;

      case 'd':
        Serial.print("one reading:\t");
        Serial.print(scale.get_units(), 1);
        Serial.print("\t| average:\t");
        Serial.println(scale.get_units(10), 5);
        break;

      case 'e':
        scale.tare();
        delay(500);
        Serial.print("one reading:\t");
        Serial.print(scale.get_units(), 1);
        Serial.print("\t| average:\t");
        Serial.println(scale.get_units(10), 5);
        break;

      case 'f':
        measureVolume();
        break;
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

void displayReset(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi");
  delay(1000);
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
