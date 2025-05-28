#include "ExampleStore.h"

// Globális példány definíció
ExampleStore exampleStore;

// Opcionális: Teszt funkciók az EEPROM rendszer ellenőrzésére
#ifdef __DEBUG

/**
 * @brief EEPROM store rendszer tesztelése (optimalizált verzió)
 *
 * Tömör teszt függvény az EEPROM store funkcionalitás ellenőrzésére.
 */
void testEepromStore() {
  DEBUG("\n=== EEPROM Store Teszt ===\n");

  // Inicializálás és alapértelmezett értékek
  StoreEepromBase<ExampleData_t>::init();
  exampleStore.loadDefaults();
  exampleStore.forceSave();
  DEBUG("Inicializálás és alapértelmezett mentés: OK\n");

  // Teszt adatok beállítása egy hívásban
  exampleStore.setName("Test Radio");
  exampleStore.setFrequency(8850);  // 88.5 MHz
  exampleStore.setVolume(75);
  exampleStore.setEnabled(true);
  exampleStore.setCalibration(1.05f);

  // Automatikus mentés és validálás
  if (exampleStore.needsSave()) {
    exampleStore.checkSave();
    DEBUG("Első mentés: OK\n");
  }

  // Második módosítás és mentés
  exampleStore.setVolume(85);
  exampleStore.setCalibration(1.10f);
  exampleStore.checkSave();

  // Validálás újrabetöltéssel
  ExampleStore testStore2;
  testStore2.load();

  bool testPassed = (exampleStore.getLastCRC() == testStore2.getLastCRC() &&
                     exampleStore.getFrequency() == testStore2.getFrequency() &&
                     exampleStore.getVolume() == testStore2.getVolume());

  DEBUG("%s - CRC: %d, Freq: %d, Vol: %d\n",
        testPassed ? "✓ TESZT SIKERES" : "✗ TESZT SIKERTELEN",
        testStore2.getLastCRC(), testStore2.getFrequency(),
        testStore2.getVolume());
  DEBUG("=== Teszt Befejezve ===\n\n");
}

/**
 * @brief Memória használat ellenőrzése (optimalizált)
 */
void checkMemoryUsage() {
  DEBUG("=== Memória ===\n");
  DEBUG("ExampleData_t: %d bájt, EEPROM igény: %d bájt\n",
        sizeof(ExampleData_t),
        StoreEepromBase<ExampleData_t>::getRequiredSize());
  DEBUG("===============\n");
}

#endif  // __DEBUG
