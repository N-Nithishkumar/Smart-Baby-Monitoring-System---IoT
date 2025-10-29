# Smart-Baby-Monitoring-System---IoT

This project help parents,

  1. To detect baby movements while sleeping, when baby woke up alert msg comes to phone

  2. When baby cries or any movements around baby happens , msg will be recieved to the respective owner.

  3. Even when baby blinks his/her eyes , the notification alert is recieved 

/***************************************************
 * Baby Monitoring System with Blynk Notifications *
 * NodeMCU (ESP8266) + DHT11 + PIR + Sound + GPS   *
 ***************************************************/
#define BLYNK_TEMPLATE_ID "TMPL30ryW04Cg"
#define BLYNK_TEMPLATE_NAME "Baby monitoring"
#define BLYNK_AUTH_TOKEN "tFuz8HvZ61QMl4M7m6OMAgQY8bdPrW9E"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// ---------- WiFi + Blynk ----------
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";   // Paste from Blynk app
char ssid[] = "Infinix";
char pass[] = "1234567890";

// ---------- Sensors ----------
#define DHTPIN D2
#define DHTTYPE DHT11
#define PIRPIN D5
#define SOUNDPIN D6

DHT dht(DHTPIN, DHTTYPE);

// ---------- GPS ----------
static const int RXPin = D8, TXPin = D7;
SoftwareSerial gpsSerial(RXPin, TXPin);
TinyGPSPlus gps;

// ---------- Timers ----------
BlynkTimer timer;

// ---------- Function Prototypes ----------
void sendSensorData();
void checkBabyMovement();
void checkBabyCry();

bool motionDetected = false;
bool cryDetected = false;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  
  pinMode(PIRPIN, INPUT);
  pinMode(SOUNDPIN, INPUT);

  dht.begin();
  Blynk.begin(auth, ssid, pass);

  // Timers
  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1000L, checkBabyMovement);
  timer.setInterval(1000L, checkBabyCry);
}

void loop() {
  Blynk.run();
  timer.run();

  while (gpsSerial.available() > 0)
    gps.encode(gpsSerial.read());
}

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT Error!");
    return;
  }

  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);

  Serial.print("Temp: "); Serial.print(t);
  Serial.print("°C  Hum: "); Serial.println(h);

  if (gps.location.isValid()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();
    String location = String("Lat: ") + String(lat, 6) + " Lon: " + String(lon, 6);
    Blynk.virtualWrite(V3, location);
  }
}

void checkBabyMovement() {
  int pirState = digitalRead(PIRPIN);

  if (pirState == HIGH && !motionDetected) {
    motionDetected = true;
    Serial.println("Baby Movement Detected!");
    Blynk.logEvent("movement_alert", "Baby Movement Detected!");
  } else if (pirState == LOW) {
    motionDetected = false;
  }
}

void checkBabyCry() {
  int soundState = digitalRead(SOUNDPIN);
  Blynk.virtualWrite(V4, soundState);   // Send sound sensor value to Blynk

  if (soundState == HIGH && !cryDetected) {
    cryDetected = true;
    Serial.println("Baby Cry Detected!");
    Blynk.logEvent("cry_alert", "Baby Cry Detected!");
  } else if (soundState == LOW) {
    cryDetected=false;
  }
}