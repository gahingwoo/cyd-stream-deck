#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <BleKeyboard.h>
#include <EEPROM.h>
#include "icons.h"

// Screen and touch parameters
#define SCREEN_W 320
#define SCREEN_H 240

#define TFT_BL 21
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_12_BIT 12
#define LEDC_BASE_FREQ 5000

// EEPROM settings
#define EEPROM_SIZE 1
#define BRIGHTNESS_ADDR 0

// Touch pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

TFT_eSPI tft = TFT_eSPI();
SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
BleKeyboard bleKeyboard("CYD StreamDeck", "Espressif", 100);

// Media key definitions
#define MEDIA_PLAY_PAUSE KEY_MEDIA_PLAY_PAUSE
#define MEDIA_NEXT_TRACK KEY_MEDIA_NEXT_TRACK
#define MEDIA_PREV_TRACK KEY_MEDIA_PREVIOUS_TRACK
#define MEDIA_VOL_UP     KEY_MEDIA_VOLUME_UP
#define MEDIA_VOL_DOWN   KEY_MEDIA_VOLUME_DOWN

#ifndef DEFAULT_BACKLIGHT_PERCENT
#define DEFAULT_BACKLIGHT_PERCENT 20
#endif

enum Page { PAGE_MAIN, PAGE_SETTINGS };
Page currentPage = PAGE_MAIN;

// Button structure
struct Button {
    int x, y, w, h;
    const uint8_t* icon;
    const uint8_t* key;
    uint16_t color;
    bool isSetting;
};

// Main page button layout
#define BTN_W (SCREEN_W / 3)
#define BTN_H (SCREEN_H / 2)
Button mainButtons[] = {
    {0, 0, BTN_W, BTN_H, icon_prev, MEDIA_PREV_TRACK, TFT_RED, false},
    {BTN_W, 0, BTN_W, BTN_H, icon_play, MEDIA_PLAY_PAUSE, TFT_BLUE, false},
    {BTN_W*2, 0, BTN_W, BTN_H, icon_next, MEDIA_NEXT_TRACK, TFT_GREEN, false},
    {0, BTN_H, BTN_W, BTN_H, icon_voldown, MEDIA_VOL_DOWN, TFT_ORANGE, false},
    {BTN_W, BTN_H, BTN_W, BTN_H, icon_settings, 0, TFT_PURPLE, true},
    {BTN_W*2, BTN_H, BTN_W, BTN_H, icon_volup, MEDIA_VOL_UP, TFT_YELLOW, false}
};

// Backlight brightness
int backlight = DEFAULT_BACKLIGHT_PERCENT;

// Save brightness to EEPROM
void saveBrightness() {
    EEPROM.write(BRIGHTNESS_ADDR, backlight);
    EEPROM.commit();
}

// Load brightness from EEPROM
void loadBrightness() {
    int savedBrightness = EEPROM.read(BRIGHTNESS_ADDR);
    if (savedBrightness > 0 && savedBrightness <= 255) {
        backlight = savedBrightness;
    }
}

// ledcAnalogWrite core
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
    uint32_t duty = (4095 / valueMax) * min(value, valueMax);
    ledcWrite(channel, duty);
}

// Draw main page with icons
void drawMainPage() {
    tft.fillScreen(TFT_BLACK);
    int iconW = 96;
    int iconH = 96;
    for (int i = 0; i < 6; i++) {
        Button &btn = mainButtons[i];
        tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
        int iconX = btn.x + (btn.w - iconW) / 2;
        int iconY = btn.y + (btn.h - iconH) / 2;
        tft.drawBitmap(iconX, iconY, btn.icon, iconW, iconH, TFT_WHITE);
    }
}

// Settings page widget parameters
#define SLIDER_X 40
#define SLIDER_Y 80
#define SLIDER_W 240
#define SLIDER_H 20
#define SLIDER_BTN_R 12

// Draw settings page (still use text for settings)
void drawSettingsPage() {
    tft.fillScreen(TFT_DARKGREY);
    // Back button
    tft.fillRect(10, 10, 60, 30, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Back", 10 + 30, 10 + 15);

    // Backlight slider
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.drawString("Backlight", SCREEN_W/2, SLIDER_Y - 20);
    tft.drawRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, TFT_WHITE);
    int sliderPos = map(backlight, 0, 255, SLIDER_X, SLIDER_X + SLIDER_W);
    tft.fillCircle(sliderPos, SLIDER_Y + SLIDER_H/2, SLIDER_BTN_R, TFT_YELLOW);

    // Pair/Unpair button
    int pairBtnY = SLIDER_Y + 50;
    tft.fillRect(SCREEN_W/2 - 60, pairBtnY, 120, 40, TFT_PURPLE);
    tft.setTextColor(TFT_WHITE, TFT_PURPLE);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    if (bleKeyboard.isConnected()) {
        tft.drawString("Unpair", SCREEN_W/2, pairBtnY + 20);
    } else {
        tft.drawString("Pair", SCREEN_W/2, pairBtnY + 20);
    }

    // Connection status
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(bleKeyboard.isConnected() ? "Connected" : "Disconnected", SCREEN_W/2, pairBtnY + 70);
}

// Main page touch handler
void handleMainPageTouch(int x, int y) {
    for (int i = 0; i < 6; i++) {
        Button &btn = mainButtons[i];
        if (x >= btn.x && x < btn.x + btn.w && y >= btn.y && y < btn.y + btn.h) {
            int iconW = 96;
            int iconH = 96;
            int iconX = btn.x + (btn.w - iconW) / 2;
            int iconY = btn.y + (btn.h - iconH) / 2;
            
            if (btn.isSetting) {
                currentPage = PAGE_SETTINGS;
                drawSettingsPage();
            } else {
                // Visual feedback - 先畫白色背景
                tft.fillRect(btn.x, btn.y, btn.w, btn.h, TFT_WHITE);
                tft.drawBitmap(iconX, iconY, btn.icon, iconW, iconH, btn.color);
                delay(100);
                // 恢復原始狀態
                tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
                tft.drawBitmap(iconX, iconY, btn.icon, iconW, iconH, TFT_WHITE);
                
                // Send media key
                if (bleKeyboard.isConnected()) {
                    bleKeyboard.write(btn.key);
                }
            }
            break;
        }
    }
}

// Settings page touch handler
void handleSettingsPageTouch(int x, int y) {
    // Back button
    if (x >= 10 && x < 70 && y >= 10 && y < 40) {
        currentPage = PAGE_MAIN;
        drawMainPage();
        return;
    }
    // Backlight slider
    if (y >= SLIDER_Y - SLIDER_BTN_R && y <= SLIDER_Y + SLIDER_H + SLIDER_BTN_R) {
        if (x >= SLIDER_X && x <= SLIDER_X + SLIDER_W) {
            backlight = map(x, SLIDER_X, SLIDER_X + SLIDER_W, 0, 255);
            ledcAnalogWrite(LEDC_CHANNEL_0, backlight);
            saveBrightness(); // Save brightness when changed
            drawSettingsPage();
            return;
        }
    }
    // Pair/Unpair button
    int pairBtnY = SLIDER_Y + 50;
    if (x >= SCREEN_W/2 - 60 && x <= SCREEN_W/2 + 60 && y >= pairBtnY && y <= pairBtnY + 40) {
        if (bleKeyboard.isConnected()) {
            bleKeyboard.end();
        } else {
            bleKeyboard.begin();
        }
        drawSettingsPage();
        return;
    }
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    loadBrightness(); // Load saved brightness
    
    tft.init();
    tft.setRotation(1); // Landscape
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);
    ts.setRotation(1);

    // Backlight PWM init
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(TFT_BL, LEDC_CHANNEL_0);
    ledcAnalogWrite(LEDC_CHANNEL_0, backlight);

    bleKeyboard.begin();
    drawMainPage();
}

void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int16_t x = map(p.x, 200, 3800, 0, SCREEN_W);
        int16_t y = map(p.y, 200, 3800, 0, SCREEN_H);
        if (currentPage == PAGE_MAIN) {
            handleMainPageTouch(x, y);
        } else if (currentPage == PAGE_SETTINGS) {
            handleSettingsPageTouch(x, y);
        }
        delay(200); // Debounce
    }
}