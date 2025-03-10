<h1 align="center">🚀 NALYTHA Project: Satellite Sculpture 🌍</h1>

## 📌 <span style="color:#ff5733;">Deskripsi Proyek</span>
**<span style="color:#3498db;">NALYTHA Project</span>** adalah sebuah **pajangan meja** berbentuk **satellite sculpture** yang dilengkapi dengan **LCD TFT 2.8" Touchscreen (240x320)** dan **ESP32-S3**. Proyek ini menampilkan informasi **waktu, cuaca, dan data lingkungan** secara real-time dengan bantuan sensor **GPS NEO-8M, BMP280, dan AHT10**, serta menggunakan **LVGL** untuk tampilan antarmuka yang lebih interaktif.

---

## 🔥 <span style="color:#27ae60;">Fitur Utama</span>
- 📟 **Tampilan Real-Time** di **LCD 2.8" TFT Touchscreen 240x320** menggunakan **LVGL** untuk UI interaktif.
- 🌡️ **Data Lingkungan**:
  - 🔥 **Suhu** (**AHT10 & BMP280**)
  - 💧 **Kelembaban udara** (**AHT10**)
  - 🏔️ **Ketinggian** (**AHT10 & BMP280**)
  - 🌪️ **Tekanan Udara** (**BMP280**)
- 📡 **Data GPS**:
  - 📍 **Latitude & Longitude** (**GPS NEO-8M**)
  - ⏳ **Waktu UTC dari satelit**
  - 🛰️ **Jumlah satelit yang terdeteksi**
  - 📶 **Kekuatan sinyal GPS**
- ☁️ **Cuaca Online** (memanfaatkan **WiFi ESP32-S3** untuk mendapatkan data dari API)
- 🖲️ **Navigasi Menu** sepenuhnya menggunakan **Touchscreen dengan LVGL**:
  - ✋ **Touchscreen interaktif** untuk navigasi dan pemilihan menu
- 🌙 **Mode Idle**: Menampilkan data utama tanpa interaksi
- ⚙️ **Pengaturan WiFi, Chat Setting, dan LCD Setting**
- 🔗 **Koneksi MQTT** untuk sinkronisasi data dengan proyek **NAT lainnya** dalam ekosistem NALYTHA

---

## 🛠️ <span style="color:#e67e22;">Hardware yang Digunakan</span>
| 📦 Komponen | 🛠️ Fungsi |
|------------|----------|
| **ESP32-S3** | Mikrocontroller utama dengan WiFi |
| **LCD TFT 2.8” Touchscreen 240x320** | Tampilan antarmuka dengan sentuhan |
| **GPS NEO-8M** | Menampilkan data lokasi dan waktu dari satelit |
| **AHT10** | Mengukur suhu, kelembaban, dan ketinggian |
| **BMP280** | Mengukur tekanan udara dan ketinggian |

---

## 🚀 <span style="color:#9b59b6;">Instalasi & Penggunaan</span>
### 1️⃣ **Persiapan Perangkat Keras**
- Hubungkan **ESP32-S3** ke **LCD TFT Touchscreen, GPS, BMP280, dan AHT10** sesuai skematik.
- Pastikan sensor bekerja dengan baik sebelum melanjutkan ke pemrograman.

### 2️⃣ **Mengunggah Kode ke ESP32-S3**
1. **Unduh Library yang Diperlukan** di Arduino IDE:
   ```cpp
   lvgl
   TFT_eSPI // Untuk mengontrol LCD
   TinyGPS++
   WiFiClientSecure
   PubSubClient // Untuk MQTT
   Adafruit_AHT10
   Adafruit_BMP280
   ```
2. **Sesuaikan Kode** dengan kredensial WiFi dan API cuaca jika diperlukan.
3. **Konfigurasi MQTT** agar dapat terhubung dengan proyek **NAT lainnya**.
4. **Upload kode ke ESP32-S3** dan pantau tampilan LCD.

### 3️⃣ **Navigasi dan Penggunaan**
- Saat dinyalakan, LCD akan menampilkan data utama secara **real-time**.
- Gunakan **touchscreen berbasis LVGL** untuk navigasi utama.
- Mode **idle** akan otomatis aktif jika tidak ada interaksi.
- Data dikirim dan diterima melalui **MQTT** untuk integrasi dengan perangkat lain dalam jaringan **NALYTHA**.

---

## 📷 <span style="color:#f1c40f;">Tampilan Antarmuka</span>
_(Tambahkan gambar tampilan LCD dan perangkat)_

---

## 🔧 <span style="color:#e74c3c;">TODO List & Pengembangan Selanjutnya</span>
- [ ] Menambahkan fitur **notifikasi cuaca ekstrem**
- [ ] Opsi **kalibrasi manual sensor**
- [ ] Integrasi dengan **sistem smart home**
- [ ] Pengembangan **sinkronisasi MQTT** yang lebih canggih

---

## 🏆 <span style="color:#2ecc71;">Kontributor</span>
- **[Nama Anda]** - Pengembang Utama

🚀 _Terima kasih telah mendukung proyek ini!_ Jika ada pertanyaan atau saran, silakan **buka issue atau pull request** di repository ini.
