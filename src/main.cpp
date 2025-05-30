/**
 * @file main.cpp
 */
#include <Arduino.h>

#include "PicoMemoryInfo.h"
#include "PicoSensorUtils.h"
#include "ScreenManager.h"
#include "SplashScreen.h"
#include "defines.h"
#include "pins.h"
#include "utils.h"

//------------------- si4735
#include <SI4735.h>
SI4735 si4735;

//------------------ TFT
#include <TFT_eSPI.h>
TFT_eSPI tft;

//------------------- Rotary Encoder
#include <RPi_Pico_TimerInterrupt.h>
RPI_PICO_Timer rotaryTimer(0); // 0-ás timer használata
#include "RotaryEncoder.h"
RotaryEncoder rotaryEncoder = RotaryEncoder(PIN_ENCODER_CLK, PIN_ENCODER_DT, PIN_ENCODER_SW, ROTARY_ENCODER_STEPS_PER_NOTCH);
#define ROTARY_ENCODER_SERVICE_INTERVAL_IN_MSEC 1 // 1msec

//-------------------- Config
#include "Config.h"
#include "StationStore.h"
#include "StoreEepromBase.h"
extern Config config;
extern FmStationStore fmStationStore;
extern AmStationStore amStationStore;

//-------------------- Screens
// Globális képernyőkezelő
ScreenManager screenManager(tft);
#include "FMSceen.h"

/**
 * @brief  Hardware timer interrupt service routine a rotaryhoz
 */
bool rotaryTimerHardwareInterruptHandler(struct repeating_timer *t) {
    rotaryEncoder.service();
    return true;
}

/**
 * @brief Core0 belépésének inicializálása.
 */
void setup() {
#ifdef __DEBUG
    Serial.begin(115200);
// Várakozás a soros port megnyitására DEBUG módban
#endif

    // PICO AD inicializálása
    PicoSensorUtils::init();

    // Beeper
    pinMode(PIN_BEEPER, OUTPUT);
    digitalWrite(PIN_BEEPER, LOW);

    // TFT LED háttérvilágítás kimenet
    pinMode(PIN_TFT_BACKGROUND_LED, OUTPUT);
    analogWrite(PIN_TFT_BACKGROUND_LED, TFT_BACKGROUND_LED_MAX_BRIGHTNESS);

    // Rotary Encoder beállítása
    rotaryEncoder.setDoubleClickEnabled(true);
    rotaryEncoder.setAccelerationEnabled(true);
    // Pico HW Timer1 beállítása a rotaryhoz
    rotaryTimer.attachInterruptInterval(ROTARY_ENCODER_SERVICE_INTERVAL_IN_MSEC * 1000, rotaryTimerHardwareInterruptHandler);

    // TFT inicializálása
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK); // Fekete háttér a splash screen-hez

#ifdef DEBUG_WAIT_FOR_SERIAL
    Utils::debugWaitForSerial(tft);
#endif

    // Csak az általános információkat jelenítjük meg először (SI4735 nélkül)
    tft.fillScreen(TFT_BLACK);

    // Program cím és build info megjelenítése
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(PROGRAM_NAME, tft.width() / 2, 20);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Version " + String(PROGRAM_VERSION), tft.width() / 2, 50);
    tft.drawString(PROGRAM_AUTHOR, tft.width() / 2, 70);

    // Build info
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Build: " + String(__DATE__) + " " + String(__TIME__), tft.width() / 2, 100);

    // Inicializálási progress
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Initializing...", tft.width() / 2, 140);

    // EEPROM inicializálása (A fordítónak muszáj megadni egy típust, itt most egy Config_t-t használunk, igaziból mindegy)
    tft.drawString("Loading EEPROM...", tft.width() / 2, 160);
    StoreEepromBase<Config_t>::init(); // Meghívjuk a statikus init metódust

    // Ha a bekapcsolás alatt nyomva tartjuk a rotary gombját, akkor töröljük a konfigot
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        tft.drawString("Reset detected...", tft.width() / 2, 180);
        Utils::beepTick();
        delay(1500);
        if (digitalRead(PIN_ENCODER_SW) == LOW) { // Ha még mindig nyomják
            tft.drawString("Loading defaults...", tft.width() / 2, 200);
            config.loadDefaults();
            Utils::beepTick();
            DEBUG("Default settings resored!\n");
        }
    } else {
        // konfig betöltése
        tft.drawString("Loading config...", tft.width() / 2, 180);
        config.load();
    }

    // Kell kalibrálni a TFT Touch-t?
    if (Utils::isZeroArray(config.data.tftCalibrateData)) {
        Utils::beepError();
        Utils::tftTouchCalibrate(tft, config.data.tftCalibrateData);
    }

    // Beállítjuk a touch scren-t
    tft.setTouch(config.data.tftCalibrateData); // Állomáslisták betöltése az EEPROM-ból (a config után!) // <-- ÚJ
    tft.drawString("Loading stations...", tft.width() / 2, 200);
    fmStationStore.load();
    amStationStore.load();

    // Splash screen megjelenítése inicializálás közben
    // Most átváltunk a teljes splash screen-re az SI4735 infókkal
    SplashScreen splash(tft);
    splash.show(true, 6); // Splash screen megjelenítése progress bar-ral

    // Lépés 1: I2C inicializálás
    splash.updateProgress(1, 6, "Initializing I2C...");
    // Az si473x (Nem a default I2C lábakon [4,5] van!!!)
    Wire.setSDA(PIN_SI4735_I2C_SDA); // I2C for SI4735 SDA
    Wire.setSCL(PIN_SI4735_I2C_SCL); // I2C for SI4735 SCL
    Wire.begin();
    delay(300);

    // Si4735 inicializálása
    splash.updateProgress(2, 6, "Detecting SI4735...");
    int16_t si4735Addr = si4735.getDeviceI2CAddress(PIN_SI4735_RESET);
    if (si4735Addr == 0) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("SI4735 NOT DETECTED!", tft.width() / 2, tft.height() / 2);
        DEBUG("Si4735 not detected");
        Utils::beepError();
        while (true) // nem megyünk tovább
            ;
    }

    // Lépés 3: SI4735 konfigurálás
    splash.updateProgress(3, 6, "Configuring SI4735...");
    si4735.setDeviceI2CAddress(si4735Addr == 0x11 ? 0 : 1); // Sets the I2C Bus Address, erre is szükség van...
    splash.drawSI4735Info(si4735);
    si4735.setAudioMuteMcuPin(PIN_AUDIO_MUTE); // Audio Mute pin
    delay(300);
    //--------------------------------------------------------------------

    // Lépés 4: Frekvencia beállítások
    splash.updateProgress(4, 6, "Setting up frequency...");
    rtv::freqstep = 1000; // hz
    rtv::freqDec = config.data.currentBFO;
    delay(200);

    // Kezdő képernyőtípus beállítása
    splash.updateProgress(5, 6, "Preparing display...");
    screenManager.switchToScreen(FMScreen::SCREEN_NAME); // A kezdő képernyő az FM képernyő
    delay(200);

    //--------------------------------------------------------------------

    // Lépés 6: Finalizálás
    splash.updateProgress(6, 6, "Starting up...");
    delay(1000); // Rövidebb delay

    // Splash screen eltűntetése
    splash.hide();

    //--------------------------------------------------------------------

    // Csippantunk egyet
    Utils::beepTick();
}

/**
 * @brief Core0 fő ciklusa.
 */
void loop() {

    //------------------- EEPROM mentés figyelése
#define EEPROM_SAVE_CHECK_INTERVAL 1000 * 60 * 5 // 5 perc
    static uint32_t lastEepromSaveCheck = 0;
    if (millis() - lastEepromSaveCheck >= EEPROM_SAVE_CHECK_INTERVAL) {
        config.checkSave();
        fmStationStore.checkSave();
        amStationStore.checkSave();
        lastEepromSaveCheck = millis();
    }
//------------------- Memória információk megjelenítése
#ifdef SHOW_MEMORY_INFO
    static uint32_t lasDebugMemoryInfo = 0;
    if (millis() - lasDebugMemoryInfo >= MEMORY_INFO_INTERVAL) {
        PicoMemoryInfo::debugMemoryInfo();
        lasDebugMemoryInfo = millis();
    }
#endif

    //------------------- Touch esemény kezelése
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    static bool lastTouchState = false;
    static uint16_t lastTouchX = 0, lastTouchY = 0;

    // Touch press esemény
    if (touched && !lastTouchState) {
        // DEBUG("Touch PRESS at (%d,%d)\n", touchX, touchY);
        TouchEvent touchEvent(touchX, touchY, true);
        screenManager.handleTouch(touchEvent);
        // bool handled = screenManager.handleTouch(touchEvent);
        //  DEBUG("Touch PRESS handled: %s\n", handled ? "YES" : "NO");
        lastTouchX = touchX;
        lastTouchY = touchY;
    }
    // Touch release esemény
    else if (!touched && lastTouchState) {
        // DEBUG("Touch RELEASE at (%d,%d)\n", lastTouchX, lastTouchY);
        TouchEvent touchEvent(lastTouchX, lastTouchY, false);
        screenManager.handleTouch(touchEvent);
        // bool handled = screenManager.handleTouch(touchEvent);
        //  DEBUG("Touch RELEASE handled: %s\n", handled ? "YES" : "NO");
    }

    lastTouchState = touched;

    //------------------- Rotary Encoder esemény kezelése

    // Rotary Encoder olvasása
    RotaryEncoder::EncoderState encoderState = rotaryEncoder.read();

    // Rotary encoder eseményeinek továbbítása a ScreenManager-nek
    if (encoderState.direction != RotaryEncoder::Direction::None || encoderState.buttonState != RotaryEncoder::ButtonState::Open) {

        // RotaryEvent létrehozása a ScreenManager típusaival
        RotaryEvent::Direction direction = RotaryEvent::Direction::None;
        if (encoderState.direction == RotaryEncoder::Direction::Up) {
            direction = RotaryEvent::Direction::Up;
        } else if (encoderState.direction == RotaryEncoder::Direction::Down) {
            direction = RotaryEvent::Direction::Down;
        }

        RotaryEvent::ButtonState buttonState = RotaryEvent::ButtonState::NotPressed;
        if (encoderState.buttonState == RotaryEncoder::ButtonState::Clicked) {
            buttonState = RotaryEvent::ButtonState::Clicked;
        } else if (encoderState.buttonState == RotaryEncoder::ButtonState::DoubleClicked) {
            buttonState = RotaryEvent::ButtonState::DoubleClicked;
        }

        // Esemény továbbítása a ScreenManager-nek
        RotaryEvent rotaryEvent(direction, buttonState);
        bool handled = screenManager.handleRotary(rotaryEvent);
        DEBUG("Rotary event handled by screen: %s\n", handled ? "YES" : "NO");
    }

    // Deferred actions feldolgozása - biztonságos képernyőváltások végrehajtása
    screenManager.processDeferredActions();

    // Képernyőkezelő loop hívása
    screenManager.loop();

    // Képernyő rajzolása (csak szükség esetén, korlátozott gyakorisággal)
    static uint32_t lastDrawTime = 0;
    const uint32_t DRAW_INTERVAL = 50; // Maximum 20 FPS (50ms között rajzolás)

    if (millis() - lastDrawTime >= DRAW_INTERVAL) {
        screenManager.draw();
        lastDrawTime = millis();
    }
}

/**
 * @brief Core1 belépésének inicializálása.
 */
void setup1() {}

/**
 * @brief Core1 fő ciklusa.
 */
void loop1() {}
