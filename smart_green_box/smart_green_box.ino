#define BLYNK_TEMPLATE_ID " ......................... "
#define BLYNK_TEMPLATE_NAME " ......................  "
#define BLYNK_AUTH_TOKEN " .......................... "

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ---- KONFIGURASI WIFI & BLYNK ----
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = " ............ ";
char pass[] = " ............ ";
 
// ---- DHT11 ----
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---- Relay ----
#define RELAY_POMPA_AIR D5
#define RELAY_POMPA_PUPUK D6
#define RELAY_UV_LAMP    D7
#define RELAY_ULTRASONIC D0

// ---- Sensor Soil & LDR ----
#define SOIL_PIN A0
#define LDR_PIN  D3

// ---- LCD ----
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---- NTP Time ----
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7*3600, 60000); // GMT+7

// ---- Variabel ----
int soilThreshold = 40;   // %

unsigned long lastPump = 0;
unsigned long lcdScrollTimer = 0;
unsigned long lcdTimeTimer = 0;

String scrollText = "";
int scrollIndex = 0;

// Status Aktuator
bool pumpAirStatus = false;
bool pumpPupukStatus = false;
bool lampUVStatus   = false;
bool ultrasonicStatus = false;

unsigned long lastPumpPupuk = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  timeClient.begin();

  pinMode(RELAY_POMPA_AIR, OUTPUT);
  pinMode(RELAY_POMPA_PUPUK, OUTPUT);
  pinMode(RELAY_UV_LAMP, OUTPUT);
  pinMode(RELAY_ULTRASONIC, OUTPUT);

  digitalWrite(RELAY_POMPA_AIR, HIGH);
  digitalWrite(RELAY_POMPA_PUPUK, HIGH);
  digitalWrite(RELAY_UV_LAMP, HIGH);
  digitalWrite(RELAY_ULTRASONIC, HIGH);

  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  lcd.init();
  lcd.backlight();

    // Teks berbeda untuk tiap baris
  String teks1 = "“Smart GreenBox: Inovasi ”";
  String teks2 = "Miftah Sholahuddin";
int panjang1 = teks1.length();
  int panjang2 = teks2.length();
  int maxPanjang = max(panjang1, panjang2);

  // Loop untuk running text kedua baris
  for (int pos = 0; pos < maxPanjang + 16; pos++) {
    lcd.clear();

    // Baris 1
    lcd.setCursor(0, 0);
    lcd.print(teks1.substring(max(0, pos - 16), min(pos, panjang1)));

    // Baris 2
    lcd.setCursor(0, 1);
    lcd.print(teks2.substring(max(0, pos - 16), min(pos, panjang2)));

    delay(300);  // atur kecepatan scrolling
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Smart Green Box");
  lcd.setCursor(0,1);
  lcd.print("MA MH TROSO");
  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  timeClient.update();

  cekPompaPupuk();

  // --- Baca Sensor ---
  int soilVal = analogRead(SOIL_PIN);
  int soilPercent = map(soilVal, 800, 300, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // LDR pakai digital
  int ldrVal = digitalRead(LDR_PIN);  // 0 = terang, 1 = gelap
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // --- Logika Pompa Air ---
  if (soilPercent < soilThreshold && millis() - lastPump > 30000) { 
    digitalWrite(RELAY_POMPA_AIR, LOW);
    pumpAirStatus = true;
    delay(5000); // siram 5 detik
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    pumpAirStatus = false;
    lastPump = millis();
  } else {
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    pumpAirStatus = false;
  }

  // --- Logika Lampu UV ---
  if (ldrVal == 1) {   // kalau gelap
    digitalWrite(RELAY_UV_LAMP, LOW);
    lampUVStatus = true;
  } else {             // kalau terang
    digitalWrite(RELAY_UV_LAMP, HIGH);
    lampUVStatus = false;
  }
  

  // --- Logika Speaker Ultrasonic (malam hari) ---
  int currentHour = timeClient.getHours();
  if (currentHour >= 18 || currentHour < 5) {
    digitalWrite(RELAY_ULTRASONIC, LOW);
    ultrasonicStatus = true;
  } else {
    digitalWrite(RELAY_ULTRASONIC, HIGH);
    ultrasonicStatus = false;
  }

  // --- Kirim ke Blynk ---
  Blynk.virtualWrite(V1, soilPercent);
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);
  Blynk.virtualWrite(V4, ldrVal);
  Blynk.virtualWrite(V5, pumpAirStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V6, pumpPupukStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V7, lampUVStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V8, ultrasonicStatus ? "ON" : "OFF");


  // --- Update LCD ---
  // Baris 1: Jam & tanggal
  if (millis() - lcdTimeTimer > 1000) {
    lcdTimeTimer = millis();

    String waktu = timeClient.getFormattedTime();
    String jamPendek = waktu.substring(0,5);      // ambil "12:34"
    time_t rawTime = timeClient.getEpochTime();
    struct tm * ti = localtime(&rawTime);
    char dateStr[11];
    snprintf(dateStr, sizeof(dateStr), "%02d/%02d/%04d", ti->tm_mday, ti->tm_mon+1, ti->tm_year+1900);

    lcd.setCursor(0,0);
    lcd.print(jamPendek + " " + String(dateStr)); // contoh "12:34 11/09/25"
  }

  // Baris 2: Running text sensor & status
  if (millis() - lcdScrollTimer > 300) { 
    lcdScrollTimer = millis();
    scrollText = "Soil:" + String(soilPercent) + 
                 "% Temp:" + String((int)temp) +(char)223 +"C" +
                 " Hum:" + String((int)hum) + "%" +
                 " Light:" + String(ldrVal ? "ON" : "OFF") +
                 " UV:" + String(lampUVStatus ? "ON" : "OFF") +
                 " Ultra:" + String(ultrasonicStatus ? "ON" : "OFF") +
                 " P.Air:" + String(pumpAirStatus ? "ON" : "OFF") +
                 " P.Pupuk:" + String(pumpPupukStatus ? "ON" : "OFF") + "   ";

    int len = scrollText.length();
    if (scrollIndex > len) scrollIndex = 0;
    String window = scrollText.substring(scrollIndex, scrollIndex+16);
    lcd.setCursor(0,1);
    lcd.print(window);
    scrollIndex++;
  }
}

// ---- Blynk manual control untuk pompa pupuk ----
BLYNK_WRITE(V10) {
  int val = param.asInt();
  if (val == 1) {
    digitalWrite(RELAY_POMPA_PUPUK, LOW);
    pumpPupukStatus = true;
    lastPumpPupuk = millis(); // simpan waktu mulai
  } else {
    digitalWrite(RELAY_POMPA_PUPUK, HIGH);
    pumpPupukStatus = false;
  }
}

void cekPompaPupuk() {
  if (pumpPupukStatus && (millis() - lastPumpPupuk > 5000)) { // auto OFF setelah 5 detik
    digitalWrite(RELAY_POMPA_PUPUK, HIGH);
    pumpPupukStatus = false;
    Blynk.virtualWrite(V10, 0); // update tombol di aplikasi Blynk
  }
}

