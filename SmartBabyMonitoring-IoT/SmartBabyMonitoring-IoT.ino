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
char auth[] = "tFuz8HvZ61QMl4M7m6OMAgQY8bdPrW9E";  // Your Blynk token
char ssid[] = "Nithish";   // Wi-Fi name
char pass[] = "Nithish12"; // Wi-Fi password

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

// ---------- Flags ----------
bool motionDetected = false;
bool cryDetected = false;

// ---------- Function Prototypes ----------
void sendSensorData();
void checkBabyMovement();
void checkBabyCry();
void checkGPSData();

void setup() {
  Serial.begin(9600);
  delay(2000);
  Serial.println("======================================");
  Serial.println("Baby Monitoring System Starting...");
  Serial.println("======================================");

  pinMode(PIRPIN, INPUT);
  pinMode(SOUNDPIN, INPUT);

  dht.begin();
  gpsSerial.begin(9600);

  Serial.println("Connecting to Wi-Fi...");
  Blynk.begin(auth, ssid, pass);
  Serial.println("Wi-Fi connected!");
  Serial.println("Connecting to Blynk...");

  // ---------- Timers ----------
  timer.setInterval(3000L, sendSensorData);
  timer.setInterval(1500L, checkBabyMovement);
  timer.setInterval(1500L, checkBabyCry);
  timer.setInterval(5000L, checkGPSData);

  Serial.println("System Setup Complete!");
}

void loop() {
  Blynk.run();
  timer.run();
}

// ---------- Send Temperature, Humidity, GPS ----------
void sendSensorData() {
  Serial.println("\n[Sensor Update]");
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("❌ DHT Reading Failed!");
    return;
  }

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("°C  |  Hum: ");
  Serial.print(h);
  Serial.println("%");

  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
}

// ---------- Check Baby Movement (PIR Sensor) ----------
void checkBabyMovement() {
  int pirState = digitalRead(PIRPIN);

  if (pirState == HIGH && !motionDetected) {
    motionDetected = true;
    Serial.println("🚼 Baby Movement Detected!");
    Blynk.logEvent("movement_alert", "Baby Movement Detected!");
  } else if (pirState == LOW) {
    motionDetected = false;
  }
}

// ---------- Check Baby Cry (Sound Sensor) ----------
void checkBabyCry() {
  int soundState = digitalRead(SOUNDPIN);
  Blynk.virtualWrite(V4, soundState);

  if (soundState == HIGH && !cryDetected) {
    cryDetected = true;
    Serial.println("😢 Baby Cry Detected!");
    Blynk.logEvent("cry_alert", "Baby Cry Detected!");
  } else if (soundState == LOW) {
    cryDetected = false;
  }
}

// ---------- Check GPS Data ----------
void checkGPSData() {
  Serial.println("[Checking GPS Data...]");
  unsigned long start = millis();

  while (millis() - start < 1000) {  // 1 second window
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
  }

  if (gps.location.isValid()) {
    float lat = gps.location.lat();
    float lon = gps.location.lng();
    Serial.print("📍 Location: ");
    Serial.print(lat, 6);
    Serial.print(", ");
    Serial.println(lon, 6);

    String location = String("Lat: ") + String(lat, 6) + " Lon: " + String(lon, 6);
    Blynk.virtualWrite(V3, location);
  } else {
    Serial.println("❌ GPS not fixed yet...");
  }
}
