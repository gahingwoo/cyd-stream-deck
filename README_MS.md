# CYD StreamDeck

Ini adalah **pengawal StreamDeck** berdasarkan **ESP32-Cheap-Yellow-Display** yang mengawal pemutaran media dan volum melalui Bluetooth. Sesuai untuk senario yang memerlukan kawalan media pantas.

![CYD StreamDeck](picture.png)

## 🌐 Pilihan Bahasa

- [English](README.md)
- [繁體中文](README_TC.md)
- [简体中文](README_CN.md)
- [粵語](README_CANTON.md)
- [Bahasa Melayu](README_MS.md)

---

## 🌟 Ciri-ciri Utama

### 📟 1. Antaramuka Paparan

📌 **Halaman Utama:**
- **Kawalan Pemutaran**: Sebelum, Main/Jeda, Seterusnya
- **Kawalan Volum**: Tambah Volum, Kurang Volum
- **Butang Tetapan**: Akses halaman tetapan

📌 **Halaman Tetapan:**
- **Pelarasan Kecerahan**: Laraskan kecerahan skrin
- **Pemadanan Bluetooth**: Butang padan/nyahpadan
- **Status Sambungan**: Papar status sambungan Bluetooth semasa

---

## 📡 2. Spesifikasi Perkakasan

- **Paparan**: Skrin Sentuh TFT 320x240
- **Sentuhan**: Pengawal Sentuh XPT2046
- **Komunikasi**: Papan Kekunci Bluetooth ESP32
- **Butang**: 6 butang maya dengan maklum balas sentuhan

---

## 🔔 3. Ciri-ciri Kawalan Media

- **Kawalan Pemutaran**:
  - Main/Jeda
  - Trek Sebelum
  - Trek Seterusnya
- **Kawalan Volum**:
  - Tambah Volum
  - Kurang Volum

---

## ⚙ 4. Ciri-ciri Automasi

- **Ingatan Kecerahan**: Mengingati tetapan kecerahan terakhir
- **Sambungan Semula Automatik**: Cuba sambung semula secara automatik apabila Bluetooth terputus
- **Maklum Balas Sentuhan**: Maklum balas visual pada tekanan butang

---

## 🚨 5. Pengendalian Ralat

- **Pemantauan Sambungan Bluetooth**: Menunjukkan "Disconnected" apabila sambungan terputus
- **Debouncing Sentuhan**: Mencegah sentuhan tidak sengaja dan pencetus berulang

---

## ⚙ Konfigurasi (PlatformIO)

Projek ini dibina dengan **PlatformIO**, menyokong pelbagai konfigurasi perkakasan. Anda boleh memilih persekitaran berbeza (`env`) untuk kompilasi berdasarkan jenis skrin anda:

- **cyd**: Untuk skrin ILI9341 (port microUSB sahaja)
- **cyd2usb**: Untuk skrin ST7789 (dengan port USB-C dan microUSB), menyokong pembalikan RGB dan pelarasan susunan warna BGR

Selain itu, fail konfigurasi `platformio.ini` termasuk pilihan yang boleh disesuaikan untuk:
- Kecerahan skrin
- TODO: Kepekaan sentuhan
- TODO: Susun atur butang

---

## 📝 Arahan Penggunaan

1. Kompil dan muat naik program ke ESP32
2. Pada penggunaan pertama, masuki halaman tetapan untuk memadankan Bluetooth
3. Selepas pemadanan berjaya, semua ciri kawalan media tersedia
4. Laraskan kecerahan atau padan semula pada bila-bila masa

---

## 🔧 Nota Pembangun

- Dibangunkan menggunakan rangka kerja Arduino
- Kebergantungan utama:
  - TFT_eSPI: Pemacu skrin
  - XPT2046_Touchscreen: Pemacu sentuhan
  - ESP32 BLE Keyboard: Fungsi papan kekunci Bluetooth 