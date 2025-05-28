#include <Arduino.h>

#include "Config.h"
#include "StationStore.h"
#include "StoreEepromBase.h"

// Globális példányok példa használatra (valós projektben ezek már deklarálva vannak)
extern Config config;
extern FmStationStore fmStationStore;
extern AmStationStore amStationStore;

/**
 * @brief Core0 belépésének inicializálása.
 */
void setup() {
    Serial.begin(115200);
    delay(2000); // Várunk a Serial kapcsolatra
}

/**
 * @brief Core0 fő ciklusa.
 */
void loop() {}

/**
 * @brief Core1 belépésének inicializálása.
 */
void setup1() {}

/**
 * @brief Core1 fő ciklusa.
 */
void loop1() {}
