#ifndef EXAMPLE_STORE_H
#define EXAMPLE_STORE_H

#include "StoreBase.h"
#include "defines.h"

/**
 * @brief Példa struktúra, amit EEPROM-ban tárolunk
 */
struct ExampleData_t {
  char szNev[32];        ///< Név string (max 31 karakter + null terminátor)
  uint16_t wFrequencia;  ///< Frekvencia Hz-ben
  uint8_t bVolume;       ///< Hangerő (0-100)
  bool bEnabled;         ///< Engedélyezett-e
  float fCalibration;    ///< Kalibrációs érték
};

/**
 * @brief Példa EEPROM store osztály
 *
 * Egyszerű használati példa a StoreBase osztály használatára.
 * Minden szükséges metódust implementál automatikus CRC kezeléssel.
 */
class ExampleStore : public StoreBase<ExampleData_t> {
 public:
  /// @brief A tárolt adat példány (public a könnyű hozzáférésért)
  ExampleData_t data;

 private:
  /// @brief EEPROM cím ahol az adat tárolódik
  static const uint16_t EEPROM_ADDRESS = 100;

 protected: /**
             * @brief A tárolt adat referenciájának visszaadása
             * @return ExampleData_t& Referencia a tárolt adatra
             */
  ExampleData_t& getData() override { return data; }

  /**
   * @brief A tárolt adat const referenciájának visszaadása
   * @return const ExampleData_t& Const referencia a tárolt adatra
   */
  const ExampleData_t& getData() const override { return data; }

  /**
   * @brief Osztály nevének visszaadása debug célokra
   * @return const char* Az osztály neve
   */
  const char* getClassName() const override { return "ExampleStore"; }

  /**
   * @brief EEPROM mentés végrehajtása (felülírva a helyes cím használatához)
   * @return uint16_t Mentett adatok CRC-je (0 ha sikertelen)
   */
  uint16_t performSave() override {
    return StoreEepromBase<ExampleData_t>::save(getData(), EEPROM_ADDRESS,
                                                getClassName());
  }

  /**
   * @brief EEPROM betöltés végrehajtása (felülírva a helyes cím használatához)
   * @return uint16_t Betöltött adatok CRC-je
   */
  uint16_t performLoad() override {
    return StoreEepromBase<ExampleData_t>::load(getData(), EEPROM_ADDRESS,
                                                getClassName());
  }

 public:
  /**
   * @brief Konstruktor - alapértelmezett értékekkel inicializál
   */
  ExampleStore() { loadDefaults(); }

  /**
   * @brief Alapértelmezett értékek beállítása
   */
  void loadDefaults() override {
    Utils::safeStrCpy(data.szNev, "Default Name");
    data.wFrequencia = 1000;
    data.bVolume = 50;
    data.bEnabled = true;
    data.fCalibration = 1.0f;

    DEBUG("[%s] Alapértelmezett értékek betöltve\n", getClassName());
  }

  // --- Getter/Setter metódusok ---

  /// @brief Név beállítása
  void setName(const char* szNewName) {
    Utils::safeStrCpy(data.szNev, szNewName);
  }

  /// @brief Név lekérdezése
  const char* getName() const { return data.szNev; }

  /// @brief Frekvencia beállítása
  void setFrequency(uint16_t wNewFreq) { data.wFrequencia = wNewFreq; }

  /// @brief Frekvencia lekérdezése
  uint16_t getFrequency() const { return data.wFrequencia; }

  /// @brief Hangerő beállítása
  void setVolume(uint8_t bNewVolume) {
    data.bVolume = (bNewVolume > 100) ? 100 : bNewVolume;
  }

  /// @brief Hangerő lekérdezése
  uint8_t getVolume() const { return data.bVolume; }

  /// @brief Engedélyezés állapot beállítása
  void setEnabled(bool bNewEnabled) { data.bEnabled = bNewEnabled; }

  /// @brief Engedélyezés állapot lekérdezése
  bool isEnabled() const { return data.bEnabled; }

  /// @brief Kalibrációs érték beállítása
  void setCalibration(float fNewCalibration) {
    data.fCalibration = fNewCalibration;
  }

  /// @brief Kalibrációs érték lekérdezése
  float getCalibration() const { return data.fCalibration; }

  /**
   * @brief Debug információk kiírása
   */
  void printDebugInfo() const {
    DEBUG("=== %s Debug Info ===\n", getClassName());
    DEBUG("Név: %s\n", data.szNev);
    DEBUG("Frekvencia: %d Hz\n", data.wFrequencia);
    DEBUG("Hangerő: %d%%\n", data.bVolume);
    DEBUG("Engedélyezve: %s\n", data.bEnabled ? "igen" : "nem");
    DEBUG("Kalibráció: %.3f\n", data.fCalibration);
    DEBUG("Jelenlegi CRC: %d\n", getCurrentCRC());
    DEBUG("Utolsó mentett CRC: %d\n", getLastCRC());
    DEBUG("Mentés szükséges: %s\n", needsSave() ? "igen" : "nem");
    DEBUG("========================\n");
  }
};

// Globális példány deklaráció (definíció a .cpp fájlban lenne)
extern ExampleStore exampleStore;

#endif  // EXAMPLE_STORE_H
