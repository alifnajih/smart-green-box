# smart-green-box

## 🔹 Fungsi Utama Program

Program ini adalah **Smart Green Box** berbasis **ESP8266 + Blynk + Sensor DHT11 + Soil Moisture + LDR + RTC/NTP + LCD I2C**.
Fungsinya:

* **Monitoring & kontrol otomatis** pompa air, pompa pupuk, lampu UV, dan alat ultrasonic.
* **Pengendalian manual** pompa pupuk lewat aplikasi **Blynk**.
* **Monitoring real-time** kelembaban tanah, suhu, kelembaban udara, intensitas cahaya, jam & tanggal.
* **LCD dengan running text** untuk menampilkan informasi sensor & status aktuator.

---

## 🔹 Struktur Hardware

1. **Sensor**

   * Soil Moisture Sensor → kelembaban tanah.
   * DHT11 → suhu & kelembaban udara.
   * LDR → kondisi terang/gelap.
   * NTPClient (online) → jam & tanggal otomatis.

2. **Aktuator (relay)**

   * Relay D5 → Pompa Air.
   * Relay D6 → Pompa Pupuk.
   * Relay D7 → Lampu UV.
   * Relay D0 → Speaker Ultrasonic (pengusir hama).

3. **Interface**

   * LCD I2C 16x2 → menampilkan jam, tanggal, running text sensor & status.
   * Blynk IoT → monitoring & kontrol jarak jauh.

---

## 🔹 Alur Program

### 1. Setup

* Koneksi ke **WiFi & Blynk**.
* Inisialisasi **sensor & LCD**.
* Menampilkan **running text intro** di LCD:

  ```
  “Smart GreenBox: Inovasi”
  Miftah Sholahuddin
  ```
* Menampilkan logo nama proyek **Smart Green Box – MA MH TROSO**.

---

### 2. Loop Utama

Setiap siklus program:

#### ✅ Baca sensor:

* **Soil Moisture** → dikonversi ke % (0–100%).
* **DHT11** → suhu & kelembaban.
* **LDR** → 0 (terang), 1 (gelap).
* **NTP Time** → jam & tanggal.

#### ✅ Logika kontrol:

* **Pompa Air**

  * Aktif otomatis kalau soilPercent < threshold (40%).
  * Hidup selama **5 detik**, lalu mati.
  * Delay 30 detik sebelum boleh nyala lagi.

* **Lampu UV**

  * Menyala saat **gelap** (LDR=1).
  * Mati saat **terang** (LDR=0).

* **Ultrasonic Speaker**

  * Hidup otomatis **malam hari** (jam ≥ 18:00 s/d < 05:00).
  * Mati di siang hari.

* **Pompa Pupuk**

  * Kontrol **manual dari Blynk (V10)**.
  * Otomatis **mati setelah 5 detik** meskipun tombol Blynk masih ON.

#### ✅ Update Blynk:

Kirim data ke dashboard:

* V1 → soil moisture (%)
* V2 → suhu (°C)
* V3 → kelembaban (%)
* V4 → status LDR
* V5 → pompa air ON/OFF
* V6 → pompa pupuk ON/OFF
* V7 → lampu UV ON/OFF
* V8 → ultrasonic ON/OFF

#### ✅ Update LCD:

* Baris 1 → Jam & tanggal (misal `12:34 30/09/2025`).
* Baris 2 → Running text semua data sensor & status aktuator (16 karakter berjalan).

---

## 🔹 Kelebihan Program

1. **Smart & otomatis**: Pompa, lampu, ultrasonic dikontrol sesuai kondisi lingkungan.
2. **Manual override**: Pompa pupuk bisa dikendalikan lewat aplikasi Blynk.
3. **Realtime monitoring**: Data sensor dikirim ke aplikasi dan LCD.
4. **Safety**: Pompa pupuk auto-off setelah 5 detik (hindari lupa mematikan).
5. **User friendly**: Ada running text di LCD + aplikasi IoT.

---

## 🔹 Contoh Output di LCD

```
12:34 30/09/2025
Soil:45% Temp:28°C ...
```

## 🔹 Contoh Data di Blynk

```
Soil Moisture : 45 %
Temperature   : 28 °C
Humidity      : 65 %
Light         : ON
Pompa Air     : OFF
Pompa Pupuk   : OFF
UV Lamp       : ON
Ultrasonic    : OFF
```

---

👉 Kesimpulan: program ini adalah **sistem kontrol otomatis pertanian mini (Smart Greenhouse/Smart Farm)** dengan **IoT monitoring** dan **kontrol lokal via LCD**.
