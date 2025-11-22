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

// LDR input pin
#define LDR_PIN 34  // <<< NEW

// EEPROM settings
#define EEPROM_SIZE 2
#define BRIGHTNESS_ADDR 0
#define AUTO_BRIGHTNESS_ADDR 1

// Touch pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Touch button positions
#define BACK_BTN_X 10
#define BACK_BTN_Y 10
#define BACK_BTN_W 60
#define BACK_BTN_H 40

#define MODE_BTN_X (SCREEN_W/2 - 60)
#define MODE_BTN_Y (SLIDER_Y + 50)
#define MODE_BTN_W 120
#define MODE_BTN_H 40

#define PAIR_BTN_X (SCREEN_W/2 - 60)
#define PAIR_BTN_Y (SLIDER_Y + 110)
#define PAIR_BTN_W 120
#define PAIR_BTN_H 40


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
bool autoBrightness = false;

// Save brightness & mode to EEPROM
void saveBrightness() {
    EEPROM.write(BRIGHTNESS_ADDR, backlight);
    EEPROM.write(AUTO_BRIGHTNESS_ADDR, autoBrightness);
    EEPROM.commit();
}

// Load brightness & mode from EEPROM
void loadBrightness() {
    int savedBrightness = EEPROM.read(BRIGHTNESS_ADDR);
    backlight = (savedBrightness > 0 && savedBrightness <= 255) ? savedBrightness : DEFAULT_BACKLIGHT_PERCENT;

    autoBrightness = EEPROM.read(AUTO_BRIGHTNESS_ADDR);
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
    tft.fillRect(BACK_BTN_X, BACK_BTN_Y, BACK_BTN_W, BACK_BTN_H, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Back", BACK_BTN_X + BACK_BTN_W/2, BACK_BTN_Y + BACK_BTN_H/2);

    // Backlight slider
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.drawString("Backlight", SCREEN_W/2, SLIDER_Y - 20);
    tft.drawRect(SLIDER_X, SLIDER_Y, SLIDER_W, SLIDER_H, autoBrightness ? TFT_LIGHTGREY : TFT_WHITE);
    int sliderPos = map(backlight, 0, 255, SLIDER_X, SLIDER_X + SLIDER_W);
    tft.fillCircle(sliderPos, SLIDER_Y + SLIDER_H/2, SLIDER_BTN_R, autoBrightness ? TFT_LIGHTGREY : TFT_YELLOW);

    // Auto/Manual toggle
    tft.fillRect(MODE_BTN_X, MODE_BTN_Y, MODE_BTN_W, MODE_BTN_H, TFT_PURPLE);
    tft.setTextColor(TFT_WHITE, TFT_PURPLE);
    tft.drawString(autoBrightness ? "Auto" : "Manual", MODE_BTN_X + MODE_BTN_W/2, MODE_BTN_Y + MODE_BTN_H/2);

    // Pair/Unpair button
    tft.fillRect(PAIR_BTN_X, PAIR_BTN_Y, PAIR_BTN_W, PAIR_BTN_H, TFT_BLUE);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString(bleKeyboard.isConnected() ? "Unpair" : "Pair", PAIR_BTN_X + PAIR_BTN_W/2, PAIR_BTN_Y + PAIR_BTN_H/2);

    // Connection status
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(bleKeyboard.isConnected() ? "Connected" : "Disconnected", SCREEN_W/2, PAIR_BTN_Y + 70);
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
    if (x >= BACK_BTN_X && x <= BACK_BTN_X + BACK_BTN_W &&
        y >= BACK_BTN_Y && y <= BACK_BTN_Y + BACK_BTN_H) {
        currentPage = PAGE_MAIN;
        drawMainPage();
        return;
    }

    // Backlight slider (manual mode only)
    if (!autoBrightness &&
        y >= SLIDER_Y - SLIDER_BTN_R && y <= SLIDER_Y + SLIDER_H + SLIDER_BTN_R &&
        x >= SLIDER_X && x <= SLIDER_X + SLIDER_W) {
        backlight = map(x, SLIDER_X, SLIDER_X + SLIDER_W, 0, 255);
        ledcAnalogWrite(LEDC_CHANNEL_0, backlight);
        saveBrightness();
        drawSettingsPage();
        return;
    }

    // Auto/Manual toggle
    if (x >= MODE_BTN_X && x <= MODE_BTN_X + MODE_BTN_W &&
        y >= MODE_BTN_Y && y <= MODE_BTN_Y + MODE_BTN_H) {
        autoBrightness = !autoBrightness;
        saveBrightness();
        drawSettingsPage();
        return;
    }

    // Pair / Unpair Bluetooth
    if (x >= PAIR_BTN_X && x <= PAIR_BTN_X + PAIR_BTN_W &&
        y >= PAIR_BTN_Y && y <= PAIR_BTN_Y + PAIR_BTN_H) {
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

    pinMode(LDR_PIN, INPUT);

    tft.init();
    tft.setRotation(1); // Landscape
    mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(mySpi);
    ts.setRotation(1);

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttachPin(TFT_BL, LEDC_CHANNEL_0);
    ledcAnalogWrite(LEDC_CHANNEL_0, backlight);

    bleKeyboard.begin();
    drawMainPage();
}

// === Auto brightness config ===
int ldrMin = 100;                 // calibrated bright value
int ldrMax = 800;                 // calibrated dark value
float smoothLdr = 0;              // for smoothing

void updateBacklight() {
    if (!autoBrightness) return;

    int ldrRaw = analogRead(LDR_PIN);          // read LDR
    smoothLdr = smoothLdr * 0.8f + ldrRaw * 0.2f;  // smoothing

    // Linear inverted mapping
    int mapped = map((int)smoothLdr, ldrMin, ldrMax, 255, 10);
    mapped = constrain(mapped, 10, 255);

    // Optional gamma curve for more natural dimming
    float ratio = (float)(smoothLdr - ldrMin) / (ldrMax - ldrMin);
    ratio = constrain(ratio, 0.0f, 1.0f);
    mapped = 10 + (int)((255 - 10) * pow(1.0 - ratio, 2.2)); // perceptual curve

    backlight = mapped;
    ledcAnalogWrite(LEDC_CHANNEL_0, backlight);

    // Optional debugging
    // Serial.print("LDR: "); Serial.print(ldrRaw);
    // Serial.print("  Smoothed: "); Serial.print(smoothLdr);
    // Serial.print("  Backlight: "); Serial.println(backlight);
}


void loop() {
    // Auto brightness mode — LDR reading
    updateBacklight();   // auto brightness update

    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int16_t x = map(p.x, 200, 3800, 0, SCREEN_W);
        int16_t y = map(p.y, 200, 3800, 0, SCREEN_H);
        Serial.print("Touch at: x=");
        Serial.print(x);
        Serial.print(", y=");
        Serial.println(y);
        if (currentPage == PAGE_MAIN) {
            handleMainPageTouch(x, y);
        } else if (currentPage == PAGE_SETTINGS) {
            handleSettingsPageTouch(x, y);
        }
        delay(200); // Debounce
    }
}