# CYD StreamDeck

A **StreamDeck controller** based on **ESP32-Cheap-Yellow-Display** that controls media playback and volume via Bluetooth. Perfect for scenarios requiring quick media control.

![CYD StreamDeck](picture.png)

## 🌐 Language Options

- [English](README.md)
- [繁體中文](README_TC.md)
- [简体中文](README_CN.md)
- [粵語](README_CANTON.md)
- [Bahasa Melayu](README_MS.md)

---

## 🌟 Core Features

### 📟 1. Display Interface

📌 **Main Page:**
- **Playback Controls**: Previous, Play/Pause, Next
- **Volume Controls**: Volume Up, Volume Down
- **Settings Button**: Access settings page

📌 **Settings Page:**
- **Brightness Adjustment**: Adjust screen brightness
- **Bluetooth Pairing**: Pair/Unpair button
- **Connection Status**: Display current Bluetooth connection status

---

## 📡 2. Hardware Specifications

- **Display**: 320x240 TFT Touchscreen
- **Touch**: XPT2046 Touch Controller
- **Communication**: ESP32 Bluetooth Keyboard
- **Buttons**: 6 virtual buttons with touch feedback

---

## 🔔 3. Media Control Features

- **Playback Control**:
  - Play/Pause
  - Previous Track
  - Next Track
- **Volume Control**:
  - Volume Up
  - Volume Down

---

## ⚙ 4. Automation Features

- **Brightness Memory**: Remembers last brightness setting
- **Auto Reconnect**: Automatically attempts to reconnect when Bluetooth disconnects
- **Touch Feedback**: Visual feedback on button press

---

## 🚨 5. Error Handling

- **Bluetooth Connection Monitoring**: Shows "Disconnected" when connection is lost
- **Touch Debouncing**: Prevents accidental touches and repeated triggers

---

## ⚙ Configuration (PlatformIO)

This project is built with **PlatformIO**, supporting multiple hardware configurations. You can select different environments (`env`) for compilation based on your screen type:

- **cyd**: For ILI9341 screens (microUSB port only)
- **cyd2usb**: For ST7789 screens (with USB-C and microUSB ports), supports RGB inversion and BGR color order adjustment

Additionally, the `platformio.ini` configuration file includes customizable options for:
- Screen brightness
- TODO: Touch sensitivity
- TODO: Button layout

---

## 📝 Usage Instructions

1. Compile and upload the program to ESP32
2. On first use, enter the settings page to pair Bluetooth
3. After successful pairing, all media control features are available
4. Adjust brightness or re-pair at any time

---

## 🔧 Developer Notes

- Developed using Arduino framework
- Main dependencies:
  - TFT_eSPI: Screen driver
  - XPT2046_Touchscreen: Touch driver
  - ESP32 BLE Keyboard: Bluetooth keyboard functionality 

## Web Flashing

You can now flash your CYD StreamDeck directly from your web browser! Visit our [Web Flashing Page](https://gahingwoo.github.io/cyd-stream-deck/webflash/index.html) to get started.

### Features
- Support for both CYD and CYD2USB models
- Multi-language support (English, Simplified Chinese, Traditional Chinese, Cantonese, Malay) on README and Webflash
- Easy-to-use webflash interface
- No need to install software to flash the firmware, you can flash CYD in the browser

### Requirements
- Chrome or Edge browser
- USB connection
