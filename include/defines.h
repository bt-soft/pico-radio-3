#ifndef DEFINES_H
#define DEFINES_H

#include <Arduino.h>

#include "pins.h"

//---- Program Information ------------------------------------------

#define PROGRAM_NAME "Pico Radio v3"
#define PROGRAM_VERSION "0.0.3"
#define PROGRAM_AUTHOR "bt-soft"

//---- Program Information ------------------------------------------

//--- Debug ---
#define __DEBUG // Debug mód vezérlése

#ifdef __DEBUG
// #define SHOW_MEMORY_INFO
#define MEMORY_INFO_INTERVAL 20 * 1000 // 20mp

// Soros portra várakozás a debug üzenetek előtt
// #define DEBUG_WAIT_FOR_SERIAL

#endif

// Feszültségmérés
#define VBUS_DIVIDER_R1 197.5f // Ellenállás VBUS és A0 között (kOhm)
#define VBUS_DIVIDER_R2 99.5f  // Ellenállás A0 és GND között (kOhm)

// Rotary Encoder
#define __USE_ROTARY_ENCODER_IN_HW_TIMER

// TFT háttérvilágítás max érték
#define TFT_BACKGROUND_LED_MAX_BRIGHTNESS 255
#define TFT_BACKGROUND_LED_MIN_BRIGHTNESS 5

//--- Battery ---
#define MIN_BATTERY_VOLTAGE 270 // Minimum akkumulátor feszültség (V*100)
#define MAX_BATTERY_VOLTAGE 405 // Maximum akkumulátor feszültség (V*100)

#define TFT_COLOR_DRAINED_BATTERY TFT_COLOR(248, 252, 0)
#define TFT_COLOR_SUBMERSIBLE_BATTERY TFT_ORANGE

//--- ScreenSaver
#define SCREEN_SAVER_TIMEOUT_MIN 1
#define SCREEN_SAVER_TIMEOUT_MAX 60
#define SCREEN_SAVER_TIMEOUT 10 // 10 perc a képernyővédő időzítése

//--- CW Decoder ---
#define CW_DECODER_DEFAULT_FREQUENCY 750 // Alapértelmezett CW dekóder frekvencia (Hz)
#define CW_DECODER_MIN_FREQUENCY 600     // Minimum CW dekóder frekvencia (Hz)
#define CW_DECODER_MAX_FREQUENCY 1500    // Maximum CW dekóder frekvencia (Hz)

//--- RTTY mód adatai
// #define RTTY_DEFAULT_MARKER_FREQUENCY 2295.0f                                      // RTTY jelölő frekvencia (Hz)
// #define RTTY_DEFAULT_SHIFT_FREQUENCY RTTY_DEFAULT_MARKER_FREQUENCY - RTTY_DEFAULT_SPACE_FREQUENCY  // RTTY eltolás frekvencia (170Hz)
// #define RTTY_DEFAULT_SPACE_FREQUENCY 2125.0f                                       // RTTY tér frekvencia (Hz)
#define RTTY_DEFAULT_MARKER_FREQUENCY 1100.0f                                                     // RTTY jelölő frekvencia (Hz)
#define RTTY_DEFAULT_SHIFT_FREQUENCY 425.0f                                                       // RTTY eltolás frekvencia (170Hz)
#define RTTY_DEFAULT_SPACE_FREQUENCY RTTY_DEFAULT_MARKER_FREQUENCY - RTTY_DEFAULT_SHIFT_FREQUENCY // RTTY space frekvencia (Hz)

//--- TFT colors ---
#define TFT_COLOR(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
// #define COMPLEMENT_COLOR(color) \
//     (TFT_COLOR((255 - ((color >> 16) & 0xFF)), (255 - ((color >> 8) & 0xFF)), (255 - (color & 0xFF))))
// #define PUSHED_COLOR(color) ((((color & 0xF800) >> 1) & 0xF800) | (((color &
// 0x07E0) >> 1) & 0x07E0) | (((color & 0x001F) >> 1) & 0x001F))
#define TFT_COLOR_BACKGROUND TFT_BLACK

//--- Array Utils ---
#define ARRAY_ITEM_COUNT(array) (sizeof(array) / sizeof(array[0]))

//--- C String compare -----
#define STREQ(a, b) (strcmp((a), (b)) == 0)

//--- Debug ---
#ifdef __DEBUG
#define DEBUG(fmt, ...) Serial.printf(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG(fmt, ...) // Üres makró, ha __DEBUG nincs definiálva
#endif

#endif // DEFINES_H