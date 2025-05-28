#include <Arduino.h>

#include "Config.h"
#include "ExampleStore.h"
#include "StationStore.h"
#include "StoreEepromBase.h"

// Globális példányok példa használatra (valós projektben ezek már deklarálva
// vannak)
extern Config config;
extern FmStationStore fmStationStore;
extern AmStationStore amStationStore;

/**
 * @brief EEPROM rendszer demonstráció
 *
 * Bemutatja az új StoreBase rendszer használatát.
 */
void demonstrateEepromSystem() {
  Serial.println("=== EEPROM Store System Demo ===");

  // 1. EEPROM rendszer inicializálása
  StoreEepromBase<Config_t>::init();
  Serial.println("EEPROM rendszer inicializálva.");

  // 2. Konfigurációs adatok betöltése
  config.load();
  Serial.println("Konfigurációs adatok betöltve.");

  // 3. FM állomások betöltése
  fmStationStore.load();
  Serial.print("FM állomások betöltve. Darabszám: ");
  Serial.println(fmStationStore.getStationCount());

  // 4. AM állomások betöltése
  amStationStore.load();
  Serial.print("AM állomások betöltve. Darabszám: ");
  Serial.println(amStationStore.getStationCount());

  // 5. Példa adatok módosítása és automatikus mentés
  config.data.tftBackgroundBrightness = 128;
  config.checkSave();  // Automatikusan menti, ha változott
  Serial.println("Konfigurációs adat módosítva és mentve.");

  // 6. Példa store objektum használata
  ExampleStore exampleStore;
  exampleStore.load();  // Adat módosítása setter metódusokkal
  exampleStore.setName("Teszt Radio");
  exampleStore.setFrequency(8850);  // 88.5 MHz (tized MHz-ben)
  exampleStore.setVolume(75);
  exampleStore.setEnabled(true);
  exampleStore.setCalibration(1.05f);

  // Automatikus mentés ha változott
  exampleStore.checkSave();
  Serial.println("Példa adat módosítva és mentve.");

  Serial.println("=== Demo befejezve ===");
}

/**
 * @brief Core0 belépésének inicializálása.
 */
void setup() {
  Serial.begin(115200);
  delay(2000);  // Várunk a Serial kapcsolatra

  Serial.println("Pico Radio v3 - EEPROM Store System");

  // EEPROM rendszer demó futtatása
  demonstrateEepromSystem();
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
